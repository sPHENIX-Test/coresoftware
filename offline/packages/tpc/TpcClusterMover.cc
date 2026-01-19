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
   * @brief Formats an Acts::Vector3 as "(x, y, z)" and writes it to an output stream.
   *
   * @param out Stream to write the formatted vector to.
   * @param v  3D vector whose components will be written in the order x, y, z.
   * @return std::ostream& Reference to the same output stream after writing.
   */
  [[maybe_unused]] std::ostream& operator<<(std::ostream& out, const Acts::Vector3& v)
  {
    out << "(" << v.x() << ", " << v.y() << ", " << v.z() << ")";
    return out;
  }
}  /**
 * @brief Construct a TpcClusterMover and initialize TPC layer geometry.
 *
 * Initializes radial spacing for the inner, middle, and outer TPC regions and
 * computes the default radius for each of the 48 readout layers as the midpoint
 * of its region bin.
 *
 * @details
 * - inner/mid/outer spacings are computed from the configured region boundary radii.
 * - layer_radius[0..15]  are set for the inner region,
 *   layer_radius[16..31] are set for the middle region,
 *   layer_radius[32..47] are set for the outer region.
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
 * @brief Populate internal layer radii from the TPC cell geometry container.
 *
 * Reads layer radii from the provided PHG4TpcGeomContainer and stores them
 * into the mover's internal layer_radius array so subsequent cluster projections
 * use the geometry-derived surface radii.
 *
 * @param cellgeo TPC geometry container providing per-layer radius information.
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
 * @brief Move TPC clusters on a track to their readout layer surfaces while leaving non‑TPC clusters unchanged.
 *
 * Fits a circle to the track's TPC cluster positions to determine the transverse trajectory and fits z as a function
 * of radius to determine longitudinal position; each TPC cluster is then projected from its distorted position
 * onto the target readout layer radius and replaced by the projected global position. Clusters for which a valid
 * projection cannot be determined are omitted from the returned list. If the track contains fewer than three TPC
 * clusters, the input list is returned unchanged.
 *
 * @param global_in Vector of (cluster key, global position) pairs for all clusters on the track. TPC clusters are
 *                  identified by their cluster key and are the only ones moved; non‑TPC clusters are preserved as provided.
 * @return std::vector<std::pair<TrkrDefs::cluskey, Acts::Vector3>> Vector of (cluster key, global position) pairs where
 *         TPC cluster positions have been replaced by their projections to the corresponding readout layer radii;
 *         non‑TPC clusters are included unchanged. Clusters that could not be projected are not included for the failed entries.
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
 * @brief Selects the intersection point between a fitted circle and a cylinder at a given radius.
 *
 * Computes the two intersection candidates between the circle (centered at (X0,Y0) with radius R)
 * and the cylinder of radius target_radius, then assigns (x,y) to the candidate chosen based on
 * proximity to the original cluster position (xclus,yclus). If the intersection computation fails,
 * the function signals the failure without modifying (x,y).
 *
 * @param target_radius Cylinder radius (target surface) to intersect with.
 * @param R Radius of the fitted circle.
 * @param X0 X coordinate of the fitted circle center.
 * @param Y0 Y coordinate of the fitted circle center.
 * @param xclus X coordinate of the original cluster position, used to select the closer intersection.
 * @param yclus Y coordinate of the original cluster position, used to select the closer intersection.
 * @param[out] x Selected intersection X coordinate.
 * @param[out] y Selected intersection Y coordinate.
 * @return int `Fun4AllReturnCodes::EVENT_OK` if a valid intersection was selected and (x,y) set;
 * `Fun4AllReturnCodes::ABORTEVENT` if the intersection calculation failed.
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