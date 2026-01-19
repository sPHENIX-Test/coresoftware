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
 * Compute an intersection coordinate between two circles.
 * 
 * @param target_radius Radius of the target circle centered at (xclus, yclus).
 * @param R Radius of the circle centered at (X0, Y0).
 * @param X0 X coordinate of the first circle center.
 * @param Y0 Y coordinate of the first circle center.
 * @param xclus X coordinate of the target circle center.
 * @param yclus Y coordinate of the target circle center.
 * @param x Output parameter set to the X coordinate of an intersection point when one exists.
 * @param y Output parameter set to the Y coordinate of an intersection point when one exists.
 * @returns Number of intersection points found: `0` (no intersection), `1` (tangent), or `2` (two intersections).
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