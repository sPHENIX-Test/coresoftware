#include "CylinderGeom_Mvtx.h"

#include "SegmentationAlpide.h"

#include <TRotation.h>
#include <TVector3.h>

#include <cmath>
#include <ostream>  // for operator<<, basic_ostream::operator<<, basic_...

using Segmentation = SegmentationAlpide;

/**
 * @brief Initialize cylinder-based MVTX geometry for a specific layer.
 *
 * Populates geometric constants and local reference positions for chips,
 * modules, half-staves, and staves (values derived from mvtx_stave_v1.gdml).
 * Also sets pixel pitch and sensor thickness from the segmentation definition.
 *
 * @param in_layer Layer index for this geometry.
 * @param in_N_staves Number of staves in the layer.
 * @param in_layer_nominal_radius Nominal radius of the layer (cm).
 * @param in_phistep Angular step between consecutive staves (radians).
 * @param in_phitilt Stave tilt angle (radians).
 * @param in_phi0 Reference phi offset for stave 0 (radians).
 */
CylinderGeom_Mvtx::CylinderGeom_Mvtx(
    int in_layer,
    int in_N_staves,
    double in_layer_nominal_radius,
    double in_phistep,
    double in_phitilt,
    double in_phi0)
  : layer(in_layer)
  , N_staves(in_N_staves)
  , N_half_staves(0)
  , layer_radius(in_layer_nominal_radius)
  , stave_phi_step(in_phistep)
  , stave_phi_tilt(in_phitilt)
  , stave_phi_0(in_phi0)
  , pixel_x(Segmentation::PitchRow)
  , pixel_z(Segmentation::PitchCol)
  , pixel_thickness(Segmentation::SensorLayerThickness)
{
  // Note that stave is centered at origin with normal to face of sensor pointing in +y direction
  // Units here are cm, same as in the gdml file

  // for all layers
  double loc_sensor_in_chip_data[3] = {0.058128, -0.0005, 0.0};  // mvtx_stave_v1.gdml

  for (int i = 0; i < 3; i++)
  {
    loc_sensor_in_chip[i] = loc_sensor_in_chip_data[i];
  }

  // inner barrel layers stave construction
  //==========================
  // from mvtx_stave_v1.gdml
  // chip 0 is the closet to connectors (-Z)
  double inner_loc_chip_in_module_data[9][3] = {
      {0.0275, -0.02075, -12.060},
      {0.0275, -0.02075, -9.0450},
      {0.0275, -0.02075, -6.0300},
      {0.0275, -0.02075, -3.0150},
      {0.0275, -0.02075, 0.0},
      {0.0275, -0.02075, 3.0150},
      {0.0275, -0.02075, 6.0300},
      {0.0275, -0.02075, 9.0450},
      {0.0275, -0.02075, 12.060}};

  double inner_loc_module_in_halfstave_data[3] = {0.0, 0.0, 0.0};  // only one module

  double inner_loc_halfstave_in_stave_data[3] = {-0.0275, 0.01825, 0.0};

  for (int i = 0; i < 3; i++)
  {
    inner_loc_module_in_halfstave[i] = inner_loc_module_in_halfstave_data[i];
    inner_loc_halfstave_in_stave[i] = inner_loc_halfstave_in_stave_data[i];
    for (int j = 0; j < 9; j++)
    {
      inner_loc_chip_in_module[j][i] = inner_loc_chip_in_module_data[j][i];
    }
  }

  return;
}

/**
 * Map a world-space position to the corresponding MVTX stave and chip indices.
 *
 * @param world Three-element vector {x, y, z} containing the point in the detector (world) coordinate frame.
 * @param stave_index Output parameter set to the stave index determined from the azimuthal angle (phi) of the world position.
 * @param chip_index Output parameter set to the chip index determined from the z coordinate of the world position (expected range 0–9).
 */
void CylinderGeom_Mvtx::get_sensor_indices_from_world_coords(std::vector<double>& world, unsigned int& stave_index, unsigned int& chip_index)
{
  // stave number is fom phi
  double phi = atan2(world[1], world[0]);
  if (phi < 0)
  {
    phi += 2.0 * M_PI;
  }

  // int stave_tmp = (int) ( (phi - stave_phi_0) / stave_phi_step );
  int stave_tmp = round((phi - stave_phi_0) / stave_phi_step);
  // std::cout << "  phi " << phi << " stave_phi_0 " << stave_phi_0 << " stave_phi_step " << stave_phi_step << " stave_tmp " << stave_tmp << std::endl;

  // sensor is from z
  double chip_delta_z = (inner_loc_chip_in_module[8][2] - inner_loc_chip_in_module[0][2]) / 8.0;
  // int chip_tmp = (int) (world[2]/chip_delta_z) + 4;  // 0-9
  int chip_tmp = round(world[2] / chip_delta_z) + 4;  // 0-9
  // std::cout << "  z " << world[2] << " chip_delta_z " << chip_delta_z << " chip_tmp " << chip_tmp << std::endl;

  stave_index = stave_tmp;
  chip_index = chip_tmp;
}

/**
 * @brief Map sensor-local coordinates to detector pixel indices, with edge clamping.
 *
 * Adjusts coordinates that lie within 5e-6 units of the active matrix edges to the nearest valid edge,
 * converts the adjusted sensor-local position into chip-local coordinates, and computes the corresponding
 * detector row and column.
 *
 * @param sensor_local Sensor-local position (TVector3).
 * @param iRow Output row index within the detector (modified on success).
 * @param iCol Output column index within the detector (modified on success).
 * @return bool `true` if the position maps to a valid pixel and `iRow`/`iCol` were set, `false` otherwise.
 */
bool CylinderGeom_Mvtx::get_pixel_from_local_coords(TVector3 sensor_local, int& iRow, int& iCol)
{
  // YCM (2020-01-02): It seems that due some round issues, local coords of hits at the edge of the sensor volume
  //                   are out by some fraction of microns from the ActiveMatrix. Making a safety check inside 0.1 um
  double EPS = 5e-6;
  if (fabs(fabs(sensor_local.X()) - SegmentationAlpide::ActiveMatrixSizeRows / 2.F) < EPS)
  {
    // std::cout << " Adjusting X,  before X= " << sensor_local.X() << std::endl;
    sensor_local.SetX(((sensor_local.X() < 0) ? -1 : 1) * (SegmentationAlpide::ActiveMatrixSizeRows / 2.F - EPS));
    // std::cout << " Adjusting X,  after X= " << sensor_local.X() << std::endl;
  }
  if (fabs(fabs(sensor_local.Z()) - SegmentationAlpide::ActiveMatrixSizeCols / 2.F) < EPS)
  {
    // std::cout << " Adjusting Z,  before Z= " << sensor_local.Z() << std::endl;
    sensor_local.SetZ(((sensor_local.Z() < 0) ? -1 : 1) * (SegmentationAlpide::ActiveMatrixSizeCols / 2.F - EPS));
    // std::cout << " Adjusting Z,  after Z= " << sensor_local.Z() << std::endl;
  }
  // YCM (2020-01-02): go from sensor to chip local coords
  TVector3 in_chip = sensor_local;
  TVector3 tr(loc_sensor_in_chip[0], loc_sensor_in_chip[1], loc_sensor_in_chip[2]);
  in_chip += tr;

  return SegmentationAlpide::localToDetector(in_chip.X(), in_chip.Z(), iRow, iCol);
}

/**
 * @brief Convert sensor-local coordinates to a linear (0-based) pixel index.
 *
 * Translates a point in sensor-local coordinates to row/column pixel indices and returns
 * the corresponding linear index computed as (col * get_NX() + row).
 *
 * @param sensor_local Point in sensor-local coordinate system.
 * @return int Linear pixel index (0-based).
 *
 * Notes:
 * - If the point lies outside the active sensor area, the function prints a diagnostic
 *   message and still returns the computed index.
 * - If the derived row or column is outside valid ranges, a diagnostic message is printed.
 */
int CylinderGeom_Mvtx::get_pixel_from_local_coords(const TVector3& sensor_local)
{
  int Ngridx;
  int Ngridz;
  bool px_in = get_pixel_from_local_coords(sensor_local, Ngridx, Ngridz);
  if (!px_in)
  {
    std::cout << PHWHERE
         << " Pixel is out sensor. ("
         << sensor_local.X() << ", "
         << sensor_local.Y() << ", "
         << sensor_local.Z() << ")."
         << std::endl;
  }

  if (Ngridx < 0 || Ngridx >= get_NX() || Ngridz < 0 || Ngridz >= get_NZ())
  {
    std::cout << PHWHERE << "Wrong pixel value X= " << Ngridx << " and Z= " << Ngridz << std::endl;
  }

  // numbering starts at zero
  return Ngridx + Ngridz * get_NX();
}

TVector3 CylinderGeom_Mvtx::get_local_coords_from_pixel(int NXZ)
{
  int Ngridz = NXZ / get_NX();
  int Ngridx = NXZ % get_NX();

  return get_local_coords_from_pixel(Ngridx, Ngridz);
}

/**
 * @brief Convert detector pixel indices to local sensor coordinates.
 *
 * Translates a pixel row/column in detector (chip) indices to the corresponding
 * position in the sensor-local coordinate system.
 *
 * @param iRow Detector row index (chip-local).
 * @param iCol Detector column index (chip-local).
 * @return TVector3 Local coordinates in the sensor frame corresponding to the pixel.
 *
 * If the provided indices are outside the detector range, a diagnostic message
 * is written to std::cout and the function still returns the transformed coordinate.
 */
TVector3 CylinderGeom_Mvtx::get_local_coords_from_pixel(int iRow, int iCol)
{
  TVector3 local;
  bool check = SegmentationAlpide::detectorToLocal((float) iRow, (float) iCol, local);
  if (!check)
  {
    std::cout << PHWHERE << "Pixel coord ( " << iRow << ", " << iCol << " )"
         << "out of range" << std::endl;
  }
  // Transform location in chip to location in sensors
  TVector3 trChipToSens(loc_sensor_in_chip[0],
                        loc_sensor_in_chip[1],
                        loc_sensor_in_chip[2]);
  local -= trChipToSens;
  return local;
}

/**
 * @brief Writes a one-line summary of this CylinderGeom_Mvtx to the provided output stream.
 *
 * The summary includes layer, layer_radius, N_staves, N_half_staves, pixel_x, pixel_z, and pixel_thickness.
 *
 * @param os Output stream that receives the summary line.
 */
void CylinderGeom_Mvtx::identify(std::ostream& os) const
{
  os << "CylinderGeom_Mvtx: layer: " << layer
     << ", layer_radius: " << layer_radius
     << ", N_staves in layer: " << N_staves
     << ", N_half_staves in layer: " << N_half_staves
     << ", pixel_x: " << pixel_x
     << ", pixel_z: " << pixel_z
     << ", pixel_thickness: " << pixel_thickness
     << std::endl;
  return;
}

// hide SegmentationAlpide include from root5 rootcint (rootcling is fine)
int CylinderGeom_Mvtx::get_NZ() const
{
  return SegmentationAlpide::NCols;
}

/**
 * @brief Get the number of sensor rows (pixel count in X / row direction).
 *
 * @return int Number of rows per sensor (NRows).
 */
int CylinderGeom_Mvtx::get_NX() const
{
  return SegmentationAlpide::NRows;
}

/**
 * @brief Get the sensor row index (X) corresponding to a linear pixel index.
 *
 * @param NXZ Linear pixel index (0-based).
 * @return int Row index (X) within [0, get_NX() - 1].
 */
int CylinderGeom_Mvtx::get_pixel_X_from_pixel_number(int NXZ) const
{
  return NXZ % get_NX();
}

/**
 * @brief Compute the Z (column) index for a pixel given its linear pixel number.
 *
 * @param NXZ Linear pixel index where pixels are numbered row-major (x varies fastest).
 * @return int Z (column) index corresponding to the linear pixel number.
 */
int CylinderGeom_Mvtx::get_pixel_Z_from_pixel_number(int NXZ) const
{
  return NXZ / get_NX();
}

/**
 * Compute the linear pixel index from x (row) and z (column) indices.
 *
 * @param xbin X (row) pixel index (0-based).
 * @param zbin Z (column) pixel index (0-based).
 * @return int Linear pixel index in row-major order: `xbin + zbin * get_NX()`.
 *
 * @deprecated Obsolete; retained for backward compatibility.
 */
int CylinderGeom_Mvtx::get_pixel_number_from_xbin_zbin(int xbin, int zbin) const  // obsolete
{
  return xbin + zbin * get_NX();
}