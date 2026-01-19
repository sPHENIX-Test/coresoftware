#ifndef MICROMEGAS_MICROMEGASCOMBINEDDATADECODER_H
#define MICROMEGAS_MICROMEGASCOMBINEDDATADECODER_H

/*!
 * \file MicromegasCombinedDataDecoder.h
 * \author Hugo Pereira Da Costa <hugo.pereira-da-costa@cea.fr>
 */

#include "MicromegasCalibrationData.h"
#include "MicromegasHotChannelMapData.h"
#include "MicromegasMapping.h"

#include <fun4all/SubsysReco.h>

#include <memory>
#include <string>

class PHCompositeNode;

/**
 * Micromegas raw data decoder component.
 * Acts as a SubsysReco module that decodes Micromegas raw hits into detector-level structures.
 */

/**
 * Construct a MicromegasCombinedDataDecoder.
 * @param name Optional module name used to identify this instance.
 */

/**
 * Perform global initialization before any runs.
 * @param topNode Pointer to the top-level PHCompositeNode.
 * @returns Status code.
 */

/**
 * Perform run-specific initialization.
 * @param topNode Pointer to the top-level PHCompositeNode.
 * @returns Status code.
 */

/**
 * Process a single event.
 * @param topNode Pointer to the top-level PHCompositeNode.
 * @returns Status code.
 */

/**
 * Finalize processing and release resources.
 * @param topNode Pointer to the top-level PHCompositeNode.
 * @returns Status code.
 */

/**
 * Set the calibration file path.
 * @param value Filesystem path to the calibration (pedestal) ROOT file.
 */

/**
 * Set the hot channel map file path.
 * @param value Filesystem path to the hot-channel map file.
 */

/**
 * Set the number of RMS sigmas used to define the static threshold for each channel.
 * @param value Threshold multiplier applied to channel RMS to determine per-channel threshold.
 */

/**
 * Set the minimum ADC value used to accept a channel regardless of pedestal/RMS.
 * This is used to exclude channels that failed calibration by requiring their raw ADC to exceed this value.
 * @param value Minimum ADC count.
 */

/**
 * Set the minimum sample index (inclusive) within a waveform to consider as part of a signal hit.
 * @param value Minimum sample index.
 */

/**
 * Set the maximum sample index (inclusive) within a waveform to consider as part of a signal hit.
 * @param value Maximum sample index.
 */
class MicromegasCombinedDataDecoder : public SubsysReco
{
 public:
  /// constructor
  MicromegasCombinedDataDecoder(const std::string& name = "MicromegasCombinedDataDecoder");

  /// global initialization
  int Init(PHCompositeNode*) override;

  /// run initialization
  int InitRun(PHCompositeNode*) override;

  /// event processing
  int process_event(PHCompositeNode*) override;

  /// end of processing
  int End(PHCompositeNode*) override;

  /// calibration file
  void set_calibration_file(const std::string& value) { m_calibration_filename = value; }

  /// hot channel map
  void set_hot_channel_map_file(const std::string& value) { m_hot_channel_map_filename = value; }

  /// set number of RMS sigma used to defined static threshold on a given channel
  void set_n_sigma(double value) { m_n_sigma = value; }

  /// set minimum ADC value, disregarding pedestal and RMS.
  /**
 * Set the minimum ADC threshold used to ignore channels when calibration has failed.
 * @param value Minimum ADC value; channels with ADC below this are considered faulty and will be removed.
 */
  void set_min_adc(double value) { m_min_adc = value; }

  /**
 * Set the minimum sample index considered part of a signal hit.
 * @param value Minimum sample index (inclusive) to treat as signal when decoding raw data.
 */
  void set_sample_min(uint16_t value) { m_sample_min = value; }

  /**
 * Set the maximum sample index considered part of a signal hit.
 * @param value Maximum sample index for signal hits.
 */
  void set_sample_max(uint16_t value) { m_sample_max = value; }

 private:
  //! raw node
  std::string m_rawhitnodename = "MICROMEGASRAWHIT";

  //!@name calibration filename
  //@{
  std::string m_calibration_filename = "TPOT_Pedestal_000.root";
  MicromegasCalibrationData m_calibration_data;
  //@}

  //!@name hot channel map
  //@{
  std::string m_hot_channel_map_filename;
  MicromegasHotChannelMapData m_hot_channels;
  //@}

  //! mapping
  MicromegasMapping m_mapping;

  /// number of RMS sigma used to define threshold
  double m_n_sigma = 5;

  //! minimum ADC value, disregarding pedestal and RMS.
  /* This removes faulty channels for which calibration has failed */
  double m_min_adc = 50;

  /// min sample for signal
  uint16_t m_sample_min = 0;

  /// max sample for signal
  uint16_t m_sample_max = 1024;

  /// keep track of number of hits per hitsetid
  using hitcountmap_t = std::map<TrkrDefs::hitsetkey, int>;
  hitcountmap_t m_hitcounts;
};

#endif