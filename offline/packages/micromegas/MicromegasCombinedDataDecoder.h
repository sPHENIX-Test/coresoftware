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
 * MicromegasCombinedDataDecoder decodes Micromegas raw data, applies calibration,
 * hot-channel masking and channel-to-channel mapping, and produces reconstructed hits.
 */

/**
 * Initialize global resources required by the decoder.
 * @param topNode Top-level node of the framework's node tree.
 * @returns `0` on success, non-zero on failure.
 */

/**
 * Perform per-run initialization (e.g., load calibration and hot-channel map).
 * @param topNode Top-level node of the framework's node tree.
 * @returns `0` on success, non-zero on failure.
 */

/**
 * Decode and process Micromegas raw data for a single event, producing calibrated
 * and mapped hits while applying thresholds and hot-channel masking.
 * @param topNode Top-level node of the framework's node tree.
 * @returns `0` on success, non-zero on failure.
 */

/**
 * Finalize processing and release any allocated resources.
 * @param topNode Top-level node of the framework's node tree.
 * @returns `0` on success, non-zero on failure.
 */

/**
 * Set the calibration file path to use for pedestal and RMS information.
 * @param value Path to the calibration file.
 */

/**
 * Set the hot-channel map file path used to identify and mask noisy channels.
 * @param value Path to the hot-channel map file.
 */

/**
 * Set the number of RMS sigmas used to compute the static per-channel threshold.
 * @param value Number of RMS sigmas.
 */

/**
 * Set the minimum ADC value (after pedestal subtraction and RMS consideration).
 * Channels with ADC below this value are treated as faulty and ignored.
 * @param value Minimum ADC threshold.
 */

/**
 * Set the minimum sample index considered when searching for signal hits.
 * @param value Minimum sample index (inclusive).
 */

/**
 * Set the maximum sample index considered when searching for signal hits.
 * @param value Maximum sample index (inclusive).
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
 * Set the minimum ADC threshold used to ignore channels with unreliable pedestal or RMS.
 * @param value Minimum ADC value; channels with pedestal or RMS below this threshold are treated as faulty and ignored.
 */
  void set_min_adc(double value) { m_min_adc = value; }

  /**
 * Set the minimum sample index considered for signal hits.
 * This value defines the lower bound (inclusive) of the sample window used when identifying signal samples.
 *
 * @param value Minimum sample index to accept as part of a signal.
 */
  void set_sample_min(uint16_t value) { m_sample_min = value; }

  /**
 * Set the maximum sample index considered when selecting signal hits.
 * @param value Maximum sample index (inclusive) to include in signal-hit selection.
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