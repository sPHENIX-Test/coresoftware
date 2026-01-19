/**
 * @file mvtx/SegmentationAlpide.cc
 * @author YCM, from Alice geom of 08/04/2019
 * @brief mvtx object with ALPIDE chip description
 */
#include "SegmentationAlpide.h"

#include <iostream>
#include <format>

/**
 * @brief Print descriptive geometry and size information for the ALPIDE segmentation to standard output.
 *
 * Prints three lines detailing:
 * - Pixel size in microns along rows and columns (with the number of rows/columns).
 * - Passive edge dimensions (bottom, top, left/right) in microns.
 * - Active and total sizes for rows and columns in centimeters.
 */
void SegmentationAlpide::print()
{
  std::cout << std::format("Pixel size: {:.2f} (along {} rows) {:.2f} (along {} columns) microns", (PitchRow * 1e4), NRows, (PitchCol * 1e4), NCols)
            << std::endl;
  std::cout << std::format("Passive edges: bottom: {:.2f}, top: {:.2f}, left/right: {:.2f} microns", (PassiveEdgeReadOut * 1e4), (PassiveEdgeTop * 1e4), (PassiveEdgeSide * 1e4))
            << std::endl;
  std::cout << std::format("Active/Total size: {:.6f}/{:.6f} (rows) {:.6f}/{:.6f} (cols) cm", ActiveMatrixSizeRows, SensorSizeRows, ActiveMatrixSizeCols, SensorSizeCols)
            << std::endl;
}