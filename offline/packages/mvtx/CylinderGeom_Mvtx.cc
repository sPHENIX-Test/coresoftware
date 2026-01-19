#include "CylinderGeom_Mvtx.h"

#include "SegmentationAlpide.h"

#include <TRotation.h>
#include <TVector3.h>

#include <cmath>
#include <ostream>  // for operator<<, basic_ostream::operator<<, basic_...

using Segmentation = SegmentationAlpide;

/**
 * @brief Constructs a CylinderGeom_Mvtx and initializes stave and sensor geometry constants.
 *
 * Initializes layer indexing, stave counts and angular parameters, sets pixel pitch and
 * sensor thickness from Segmentation constants, and populates sensor/chip/module/half-stave
 * localization offsets (values derived from mvtx_stave_v1.gdml).
 *
 * All spatial values are expressed in centimeters.
 *
 * @param in_layer Layer index for this geometry.
 * @param in_N_staves Number of staves in the layer.
 * @param in_layer_nominal_radius Nominal radius of the layer.
 * @param in_phistep Angular step between staves (phi).
 * @param in_phitilt Stave tilt angle about the phi direction.
 * @param in_phi0 Reference phi offset for stave indexing.
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
 * @brief Compute stave and chip indices corresponding to a 3D point in world coordinates.
 *
 * Calculates the stave index from the point's azimuthal angle (phi) around the cylinder
 * and the chip index from the point's axial (z) position, then stores them in the
 * provided output parameters.
 *
 * @param world A 3-element vector [x, y, z] giving the point in world coordinates.
 * @param stave_index Output parameter set to the computed stave index (azimuthal).
 * @param chip_index Output parameter set to the computed chip index (axial/z).
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
 * @brief Map sensor-local coordinates to detector pixel indices.
 *
 * Converts a point given in sensor-local coordinates to chip-local coordinates,
 * assigns the corresponding detector row and column, and returns whether the
 * point maps to a valid pixel. Coordinates very close to the sensor edges are
 * nudged slightly to avoid rounding issues.
 *
 * @param sensor_local Sensor-local position (X and Z components are used).
 * @param iRow Output: detector row index corresponding to the X direction.
 * @param iCol Output: detector column index corresponding to the Z direction.
 * @return `true` if the coordinates map to a valid pixel and `iRow`/`iCol` are set, `false` otherwise.
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
 * @brief Convert sensor-local coordinates to a linear pixel index.
 *
 * @param sensor_local Coordinates expressed in the sensor-local frame.
 * @return int Linear pixel index computed as (column) + (row) * get_NX(); indexing starts at 0.
 *
 * If the coordinates lie outside the active sensor area, the function prints diagnostic messages
 * and still returns the computed linear index, which may be outside the valid [0, get_NX()*get_NZ()) range.
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
 * @brief Convert detector pixel indices to sensor-local coordinates.
 *
 * Converts the given detector pixel indices (row, column) into a TVector3
 * representing the position in the sensor-local coordinate system.
 *
 * @param iRow Pixel row index within the detector.
 * @param iCol Pixel column index within the detector.
 * @return TVector3 Position in sensor-local coordinates corresponding to the pixel.
 *
 * Note: If the provided indices are out of range, a diagnostic message is
 * printed to stdout and the returned vector is the computed value after the
 * failed conversion attempt (offset from chip to sensor coordinates is still applied).
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
 * @brief Prints a concise description of this cylinder geometry to the given stream.
 *
 * Outputs layer index, layer radius, number of full and half staves, pixel X and Z pitch, and pixel thickness
 * in a single line to the provided output stream.
 *
 * @param os Output stream to receive the summary.
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
 * @brief Number of pixel rows in the detector's X dimension.
 *
 * @return int Number of X (row) pixels per sensor/chip.
 */
int CylinderGeom_Mvtx::get_NX() const
{
  return SegmentationAlpide::NRows;
}

/**
 * @brief Compute the X (row) index corresponding to a linear pixel index.
 *
 * @param NXZ Linear pixel index encoded as x + z * get_NX().
 * @return int X index within the pixel grid in the range [0, get_NX() - 1].
 */
int CylinderGeom_Mvtx::get_pixel_X_from_pixel_number(int NXZ) const
{
  return NXZ % get_NX();
}

/**
 * @brief Compute the Z (column) index for a given linear pixel number.
 *
 * @param NXZ Linear pixel index in row-major order (x + z * get_NX()).
 * @return int Z (column) index corresponding to NXZ.
 */
int CylinderGeom_Mvtx::get_pixel_Z_from_pixel_number(int NXZ) const
{
  return NXZ / get_NX();
}

/**
 * @brief Compute the linear pixel index from x and z bin coordinates.
 *
 * @param xbin Zero-based x (row) index within the sensor grid.
 * @param zbin Zero-based z (column) index within the sensor grid.
 * @return int Linear pixel index equal to xbin + zbin * get_NX().
 */
int CylinderGeom_Mvtx::get_pixel_number_from_xbin_zbin(int xbin, int zbin) const  // obsolete
{
  return xbin + zbin * get_NX();
}
