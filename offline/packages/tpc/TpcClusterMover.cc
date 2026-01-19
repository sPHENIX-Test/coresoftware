/*!
 * \file TpcClusterMover.cc
 * \brief moves distortion corrected clusters back to their TPC surface
 * \author Tony Frawley, April 2022
 */

#include "TpcClusterMover.h"

#include <fun4all/Fun4AllReturnCodes.h>
#include <trackbase/TrackFitUtils.h>

#include <g4detectors/PHG4TpcGeom.h>
#include <g4detectors/PHG4TpcGeomContainer.h>
#include <climits>
#include <cmath>
#include <iostream>

namespace
{
  /**
   * @brief Writes an Acts::Vector3 to an output stream as a parenthesized, comma-separated tuple.
   *
   * Formats the vector as "(x, y, z)" where x, y, and z are the vector components.
   *
   * @param out Output stream to write to.
   * @param v Vector whose components will be formatted.
   * @return std::ostream& Reference to the same output stream to allow chaining.
   */
  [[maybe_unused]] std::ostream& operator<<(std::ostream& out, const Acts::Vector3& v)
  {
    out << "(" << v.x() << ", " << v.y() << ", " << v.z() << ")";
    return out;
  }
}  /**
 * @brief Initialize spacings and nominal radii for TPC readout layers.
 *
 * Computes the radial spacing for the inner, mid, and outer TPC regions and
 * populates the layer_radius array with the nominal radius (layer center)
 * for 48 TPC layers (16 layers per region).
 *
 * The spacing for each region is derived from its configured min/max radii,
 * and each region's 16 layer entries are set to the layer center:
 * - layer_radius[0..15]   : inner region (from inner_tpc_min_radius)
 * - layer_radius[16..31]  : mid region   (from mid_tpc_min_radius)
 * - layer_radius[32..47]  : outer region (from outer_tpc_min_radius)
 */

TpcClusterMover::TpcClusterMover()
  : inner_tpc_spacing((mid_tpc_min_radius - inner_tpc_min_radius) / 16.0)
  , mid_tpc_spacing((outer_tpc_min_radius - mid_tpc_min_radius) / 16.0)
  , outer_tpc_spacing((outer_tpc_max_radius - outer_tpc_min_radius) / 16.0)
{
  // initialize layer radii

  for (int i = 0; i < 16; ++i)
  {
    layer_radius[i] = inner_tpc_min_radius + (double) i * inner_tpc_spacing + 0.5 * inner_tpc_spacing;
  }
  for (int i = 0; i < 16; ++i)
  {
    layer_radius[i + 16] = mid_tpc_min_radius + (double) i * mid_tpc_spacing + 0.5 * mid_tpc_spacing;
  }
  for (int i = 0; i < 16; ++i)
  {
    layer_radius[i + 32] = outer_tpc_min_radius + (double) i * outer_tpc_spacing + 0.5 * outer_tpc_spacing;
  }
}

/**
 * @brief Populate the internal TPC layer radii from a geometry container.
 *
 * Updates this object's layer_radius[] entries with the radius values taken
 * from the provided PHG4TpcGeomContainer in the container's iteration order.
 *
 * @param cellgeo Geometry container providing per-layer radius information.
 */
void TpcClusterMover::initialize_geometry(PHG4TpcGeomContainer* cellgeo)
{
  if (_verbosity > 0)
  {
    std::cout << "TpcClusterMover: Initializing layer radii for Tpc from cell geometry object" << std::endl;
  }

  int layer = 0;
  PHG4TpcGeomContainer::ConstRange layerrange = cellgeo->get_begin_end();
  for (PHG4TpcGeomContainer::ConstIterator layeriter = layerrange.first;
       layeriter != layerrange.second;
       ++layeriter)
  {
    layer_radius[layer] = layeriter->second->get_radius();
    layer++;
  }
}

/**
 * @brief Move TPC clusters on a track radially to their readout-layer surface positions.
 *
 * Fits a circle to the track's TPC cluster XY positions and a line to z versus radius, then projects each TPC
 * cluster along the fitted circle from its current radius to the layer's configured readout radius and adjusts z
 * according to the fitted line. Non-TPC clusters are preserved unchanged.
 *
 * @return std::vector<std::pair<TrkrDefs::cluskey, Acts::Vector3>> A vector of cluster keys and their updated global
 * positions. Non-TPC clusters appear exactly as in the input. TPC clusters are replaced by their projected positions;
 * TPC clusters for which geometric intersection fails are omitted. If the track contains fewer than three TPC clusters,
 * the original input vector is returned unchanged.
 */
std::vector<std::pair<TrkrDefs::cluskey, Acts::Vector3>> TpcClusterMover::processTrack(const std::vector<std::pair<TrkrDefs::cluskey, Acts::Vector3>>& global_in)
{
  // Get the global positions of the TPC clusters for this track, already corrected for distortions, and move them to the surfaces
  // The input object contains all clusters for the track

  std::vector<std::pair<TrkrDefs::cluskey, Acts::Vector3>> global_moved;

  std::vector<Acts::Vector3> tpc_global_vec;
  std::vector<TrkrDefs::cluskey> tpc_cluskey_vec;

  for (const auto& [ckey, global] : global_in)
  {
    const auto trkrid = TrkrDefs::getTrkrId(ckey);
    if (trkrid == TrkrDefs::tpcId)
    {
      tpc_cluskey_vec.push_back(ckey);
      tpc_global_vec.push_back(global);
    }
    else
    {
      // si clusters stay where they are
      global_moved.emplace_back(ckey, global);
    }
  }

  // need at least 3 clusters to fit a circle
  if (tpc_global_vec.size() < 3)
  {
    if (_verbosity > 0)
    {
      std::cout << "  -- skip this tpc track, not enough clusters: " << tpc_global_vec.size() << std::endl;
    }
    return global_in;
  }

  // fit a circle to the TPC clusters
  const auto [R, X0, Y0] = TrackFitUtils::circle_fit_by_taubin(tpc_global_vec);

  // get the straight line representing the z trajectory in the form of z vs radius
  const auto [A, B] = TrackFitUtils::line_fit(tpc_global_vec);

  // Now we need to move each TPC cluster associated with this track to the readout layer radius
  for (unsigned int i = 0; i < tpc_global_vec.size(); ++i)
  {
    TrkrDefs::cluskey cluskey = tpc_cluskey_vec[i];
    unsigned int layer = TrkrDefs::getLayer(cluskey);
    Acts::Vector3 global = tpc_global_vec[i];

    // get circle position at target surface radius
    double target_radius = layer_radius[layer - 7];
    int ret = get_circle_circle_intersection(target_radius, R, X0, Y0, global[0], global[1], _x_proj, _y_proj);
    if (ret == Fun4AllReturnCodes::ABORTEVENT)
    {
      continue;  // skip to next cluster
    }
    // z projection is unique
    _z_proj = B + A * target_radius;

    // get circle position at cluster radius
    double cluster_radius = sqrt(global[0] * global[0] + global[1] * global[1]);
    ret = get_circle_circle_intersection(cluster_radius, R, X0, Y0, global[0], global[1], _x_start, _y_start);
    if (ret == Fun4AllReturnCodes::ABORTEVENT)
    {
      continue;  // skip to next cluster
    }
    // z projection is unique
    _z_start = B + A * cluster_radius;

    // calculate dx, dy, dz along circle trajectory from cluster radius to surface radius
    double xnew = global[0] - (_x_start - _x_proj);
    double ynew = global[1] - (_y_start - _y_proj);
    double znew = global[2] - (_z_start - _z_proj);

    // now move the cluster to the surface radius
    // we keep the cluster key fixed, change the surface if necessary

    Acts::Vector3 global_new(xnew, ynew, znew);

    // add the new position and surface to the return object
    global_moved.emplace_back(cluskey, global_new);

    if (_verbosity > 2)
    {
      std::cout << "Cluster " << cluskey << " xstart " << _x_start << " xproj " << _x_proj << " ystart " << _y_start << " yproj " << _y_proj
                << " zstart " << _z_start << " zproj " << _z_proj << std::endl;
      std::cout << " layer " << layer << " layer radius " << target_radius << " cluster radius " << cluster_radius << std::endl;
      std::cout << "  global in " << global[0] << "  " << global[1] << "  " << global[2] << std::endl;
      std::cout << "  global new " << global_new[0] << "  " << global_new[1] << "  " << global_new[2] << std::endl;
    }
  }

  return global_moved;
}

/**
 * @brief Compute the intersection point between a fitted circle and a cylinder at a given radius.
 *
 * Selects the intersection point (x,y) on the circle defined by (R, X0, Y0) that is closest to the cluster's
 * current (xclus, yclus). If the geometric intersection computation fails, the call indicates the cluster should
 * be skipped.
 *
 * @param target_radius Radius of the cylinder (distance from z-axis) to intersect with.
 * @param R Radius of the fitted circle.
 * @param X0 X-coordinate of the fitted circle center.
 * @param Y0 Y-coordinate of the fitted circle center.
 * @param xclus X-coordinate of the cluster's current position (used to choose the correct intersection).
 * @param yclus Y-coordinate of the cluster's current position (used to choose the correct intersection).
 * @param[out] x Chosen intersection X-coordinate (set on success).
 * @param[out] y Chosen intersection Y-coordinate (set on success).
 * @return int Fun4AllReturnCodes::EVENT_OK on success; Fun4AllReturnCodes::ABORTEVENT if the intersection
 * calculation failed and the cluster should be skipped.
 */
int TpcClusterMover::get_circle_circle_intersection(double target_radius, double R, double X0, double Y0, double xclus, double yclus, double& x, double& y) const
{
  // finds the intersection of the fitted circle with the cylinder having radius = target_radius
  const auto [xplus, yplus, xminus, yminus] = TrackFitUtils::circle_circle_intersection(target_radius, R, X0, Y0);

  // We only need to check xplus for failure, skip this TPC cluster in that case
  if (std::isnan(xplus))
  {
    {
      if (_verbosity > 1)
      {
        std::cout << " circle/circle intersection calculation failed, skip this cluster" << std::endl;
        std::cout << " target_radius " << target_radius << " fitted R " << R << " fitted X0 " << X0 << " fitted Y0 " << Y0 << std::endl;
      }
    }
    return Fun4AllReturnCodes::ABORTEVENT;  // skip to next cluster
  }

  // we can figure out which solution is correct based on the cluster position in the TPC
  if (fabs(xclus - xplus) < 5.0 && fabs(yclus - yplus) < 5.0)  // 5 cm, large and arbitrary
  {
    x = xplus;
    y = yplus;
  }
  else
  {
    x = xminus;
    y = yminus;
  }
  return Fun4AllReturnCodes::EVENT_OK;
}