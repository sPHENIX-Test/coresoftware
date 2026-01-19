#ifndef TPC_TPCCLUSTERMOVER_H
#define TPC_TPCCLUSTERMOVER_H

/*!
 * \file TpcClusterMover.h
 * \Moves TPC clusters to the readout TPC surface after distortion corrections
 * \author Tony Frawley, May 2022
 */
#include <trackbase/TrkrDefs.h>
#include <trackbase_historic/ActsTransformations.h>
#include <vector>

class PHG4TpcGeomContainer;

/**
 * Construct a TpcClusterMover.
 */

/**
 * Set the internal verbosity level for diagnostic output.
 * @param verb Verbosity level (higher values increase detail).
 */

/**
 * Project track cluster positions onto the TPC readout surface.
 * @param global_in Vector of pairs (cluster key, global position/orientation) representing input cluster coordinates.
 * @returns Vector of pairs with the same cluster keys and transformed positions on the readout surface.
 */

/**
 * Update the internal geometry parameters to match the provided TPC cell geometry container.
 * @param cellgeo Pointer to the PHG4TpcGeomContainer containing the TPC cell geometry to use.
 */

/**
 * Compute an intersection point between two circles: one centered at (X0,Y0) with radius R,
 * and the circle centered at (xclus,yclus) with radius inferred from target_radius.
 * The intersection coordinates are written to x and y.
 * @param target_radius Target radius used for determining the intersection circle.
 * @param R Radius of the first circle centered at (X0, Y0).
 * @param X0 X coordinate of the first circle center.
 * @param Y0 Y coordinate of the first circle center.
 * @param xclus X coordinate of the second circle center (cluster position).
 * @param yclus Y coordinate of the second circle center (cluster position).
 * @param x Output X coordinate of the intersection point.
 * @param y Output Y coordinate of the intersection point.
 * @returns `0` on success (intersection found), non-zero on failure (no intersection or error).
 */
class TpcClusterMover
{
 public:
  //! constructor
  TpcClusterMover();

  void set_verbosity(int verb) { _verbosity = verb; }

  std::vector<std::pair<TrkrDefs::cluskey, Acts::Vector3>> processTrack(const std::vector<std::pair<TrkrDefs::cluskey, Acts::Vector3>>& global_in);

  //! Updates the assumed default geometry below to that contained in the
  //! cell geo
  void initialize_geometry(PHG4TpcGeomContainer *cellgeo);

 private:
  int get_circle_circle_intersection(double target_radius, double R, double X0, double Y0, double xclus, double yclus, double &x, double &y) const;

  double _z_start = 0.0;
  double _y_start = 0.0;
  double _x_start = 0.0;

  double _z_proj = 0.0;
  double _y_proj = 0.0;
  double _x_proj = 0.0;

  double layer_radius[48] = {0};
  double inner_tpc_min_radius = 30.0;
  double mid_tpc_min_radius = 40.0;
  double outer_tpc_min_radius = 60.0;
  double outer_tpc_max_radius = 76.4;

  double inner_tpc_spacing = 0.0;
  double mid_tpc_spacing = 0.0;
  double outer_tpc_spacing = 0.0;

  int _verbosity = 0;
};

#endif