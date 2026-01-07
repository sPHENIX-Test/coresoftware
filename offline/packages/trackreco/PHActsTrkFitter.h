/*!
 *  \file		PHActsTrkFitter.h
 *  \brief		Refit SvtxTracks with Acts.
 *  \details	        Refit SvtxTracks with Acts
 *  \author		Joe Osborn, Tony Frawley <afrawley@fsu.edu>
 */

#ifndef TRACKRECO_ACTSTRKFITTER_H
#define TRACKRECO_ACTSTRKFITTER_H

#include "ActsAlignmentStates.h"
#include "ActsEvaluator.h"

#include <fun4all/SubsysReco.h>

#include <trackbase/ActsSourceLink.h>
#include <trackbase/ActsTrackFittingAlgorithm.h>

#include <tpc/TpcGlobalPositionWrapper.h>

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/Utilities/BinnedArray.hpp>
#include <Acts/Utilities/Helpers.hpp>
#include <Acts/Utilities/Logger.hpp>

#include <ActsExamples/EventData/Trajectories.hpp>

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <memory>
#include <string>

class alignmentTransformationContainer;
class ActsGeometry;
class SvtxTrack;
class SvtxTrackMap;
class TrackSeed;
class TrackSeedContainer;
class TrkrClusterContainer;
class SvtxAlignmentStateMap;
class PHG4TpcGeomContainer;

using SourceLink = ActsSourceLink;
using FitResult = ActsTrackFittingAlgorithm::TrackFitterResult;
using Trajectory = ActsExamples::Trajectories;
using Measurement = Acts::Measurement<Acts::BoundIndices, 2>;
using SurfacePtrVec = std::vector<const Acts::Surface*>;
using SourceLinkVec = std::vector<Acts::SourceLink>;

/**
   * Construct a PHActsTrkFitter instance.
   * @param name Instance name used for messaging and node registration.
   */
  /**
   * Finalize module work, write and close any open files, and perform cleanup.
   * @param topNode Top-level node of the current event/node tree.
   * @returns 0 on success, non-zero on failure.
   */
  /**
   * Acquire required nodes and create any missing nodes needed for the run.
   * @param topNode Top-level node of the current event/node tree.
   * @returns 0 on success, non-zero on failure.
   */
  /**
   * Process a single event by running the track refit workflow.
   * @param topNode Top-level node of the current event/node tree.
   * @returns 0 on success, non-zero on failure.
   */
  /**
   * Reset per-event internal state.
   * @param topNode Top-level node of the current event/node tree.
   * @returns 0 on success, non-zero on failure.
   */
  /**
   * Enable or disable internal timing instrumentation.
   * @param timeAnalysis If true, enable timing analysis; if false, disable it.
   */
  /**
   * Enable or disable use of the directed navigator to fit tracks containing silicon and Micromegas hits.
   * @param fitSiliconMMs If true, use direct navigation for silicon+MM fits.
   */
  /**
   * When using direct navigation, force fitting to use only silicon surfaces (ignore MM surfaces).
   * @param forceSiOnlyFit If true, restrict direct-navigation fits to silicon surfaces only.
   */
  /**
   * Require Micromegas hits to be present when performing Silicon+MM direct-navigation fits.
   * @param value If true, require Micromegas presence for SiliconMM fits.
   */
  /**
   * Mark silicon clusters to be ignored during fitting.
   */
   /**
   * Control whether SvtxTrackState entries are updated from Acts fit results.
   * @param fillSvtxTrackStates If true, update SvtxTrackState information on tracks.
   */
  /**
   * Enable or disable the Acts-based evaluator.
   * @param actsEvaluator If true, enable evaluator output.
   */
  /**
   * Enable or disable the Acts-based evaluator in simulation mode; also enables the evaluator flag.
   * @param actsEvaluator If true, enable simulation-mode evaluator and set evaluator enabled.
   */
  /**
   * Set the evaluator output filename.
   * @param name Output filename for the evaluator.
   */
  /**
   * Configure an external magnetic field map by path/name.
   * @param fieldMap Identifier or path to the field map to use.
   */
  /**
   * Set the absolute PDG hypothesis used as the particle assumption for fitting.
   * @param pHypothesis PDG code to assume (absolute value).
   */
  /**
   * Toggle commissioning mode which may alter fitter behavior or tolerances.
   * @param com If true, enable commissioning mode.
   */
  /**
   * Enable or disable the chi^2-based outlier finder inside the fitter.
   * @param outlier If true, enable outlier finding.
   */
  /**
   * Specify an output file for the outlier finder results.
   * @param outfilename Path to the outlier finder output file.
   */
  /**
   * Set the number of fitter iterations to perform.
   * @param iter Number of iterations.
   */
  /**
   * Set the output Svtx track map name.
   * @param map_name Name to use for the track map.
   */
  /**
   * Set the Svtx seed map name.
   * @param map_name Name to use for the seed map.
   */
  /**
   * Set the Svtx alignment state map name and configure alignment state handling.
   * @param map_name Name to use for the SvtxAlignmentStateMap.
   */
  /**
   * Configure running mode for pp collisions.
   * @param ispp If true, enable pp running mode.
   */
  /**
   * Enable or disable geometric crossing estimate usage.
   * @param flag If true, enable geometric crossing estimate.
   */
  /**
   * Enable or disable use of the cluster mover facility.
   * @param use If true, enable cluster mover.
   */
  /**
   * Mark a detector layer to be ignored during fits.
   * @param layer Integer identifier of the layer to ignore.
   */
  /**
   * Set the name of the TRKR_CLUSTER container to read clusters from.
   * @param name Name of the TRKR_CLUSTER container.
   */
  /**
   * Enable or disable direct navigation mode.
   * @param flag If true, use direct navigation when available.
   */
  /**
   * Retrieve required nodes from the node tree and cache pointers to them.
   * @param topNode Top-level node of the current event/node tree.
   * @returns 0 on success, non-zero on failure.
   */
  /**
   * Create module-specific nodes under the provided top node when absent.
   * @param topNode Top-level node of the current event/node tree.
   * @returns 0 on success, non-zero on failure.
   */
  /**
   * Iterate over tracks and perform fitting actions at the given Acts logging level.
   * @param logLevel Logging threshold passed to Acts components during fitting.
   */
  /**
   * Translate Acts trajectory output into SvtxTrack fields and SvtxTrackState entries.
   * @param tips Indices of trajectory tips to consider for state extraction.
   * @param paramsMap Map of indexed trajectory parameters.
   * @param tracks Container of Acts fit track objects produced by the fitter.
   * @param track SvtxTrack to be updated with fit results.
   */
  /**
   * Fit a single track using either regular navigation or direct navigation based on configuration.
   * @param sourceLinks Source links (measurements) that compose the track to be fitted.
   * @param seed Initial seed parameters for the fitter.
   * @param kfOptions General fitter options and algorithm configuration.
   * @param surfSequence Ordered sequence of surface pointers for direct navigation (may be empty for regular navigation).
   * @param calibrator Calibrator adapter used to convert measurements for fitting.
   * @param tracks Output container that will be populated with resulting Acts tracks.
   * @returns Result object describing fit success, status, and produced measurements.
   */
  /**
   * Build a sorted vector of surface pointers corresponding to the provided source links for direct navigation.
   * @param sourceLinks Input source links.
   * @param surfaces Output vector populated with corresponding surface pointers in navigation order.
   * @returns A possibly filtered vector of source links corresponding to the constructed surfaces.
   */
  /**
   * Validate the integrity and ordering of a surface vector prepared for direct navigation.
   * @param surfaces Surface pointer vector to validate.
   */
  /**
   * Convert a fit result into SvtxTrack updates and determine fit acceptance.
   * @param fitOutput Fit result produced by the Acts fitter.
   * @param seed Original TrackSeed corresponding to the fit.
   * @param track SvtxTrack to update with the fit results.
   * @param tracks Container of Acts track objects produced during fitting.
   * @param measurements Container of measurements produced by the fit.
   * @returns `true` if the fit result was successfully converted and accepted, `false` otherwise.
   */
  /**
   * Provide a default covariance matrix to be used when a seed lacks covariance information.
   * @returns Default bound covariance matrix sized for the track parameterization used by the fitter.
   */
  /**
   * Print diagnostic information for a track seed to the configured logger.
   * @param seed Seed parameters to be logged.
   */
  /**
   * Collect surfaces that contain material into an internal list if they are not already present.
   * @param surface Test surface to inspect and possibly add to the selector's list.
   */
  class PHActsTrkFitter : public SubsysReco
{
 public:
  /// Default constructor
  PHActsTrkFitter(const std::string& name = "PHActsTrkFitter");

  /// Destructor
  ~PHActsTrkFitter() override = default;

  /// End, write and close files
  int End(PHCompositeNode* topNode) override;

  /// Get and create nodes
  int InitRun(PHCompositeNode* topNode) override;

  /// Process each event by calling the fitter
  int process_event(PHCompositeNode* topNode) override;

  int ResetEvent(PHCompositeNode* topNode) override;

  /// Do some internal time benchmarking analysis
  void doTimeAnalysis(bool timeAnalysis) { m_timeAnalysis = timeAnalysis; }

  /// Run the direct navigator to fit only tracks with silicon+MM hits
  void fitSiliconMMs(bool fitSiliconMMs)
  {
    m_fitSiliconMMs = fitSiliconMMs;
  }

  /// with direct navigation, force a fit with only silicon hits
  void forceSiOnlyFit(bool forceSiOnlyFit)
  {
    m_forceSiOnlyFit = forceSiOnlyFit;
  }

  /// require micromegas in SiliconMM fits
  void setUseMicromegas(bool value)
  {
    m_useMicromegas = value;
  }
  void ignoreSilicon()
  {
    m_ignoreSilicon = true;
  }
  void setUpdateSvtxTrackStates(bool fillSvtxTrackStates)
  {
    m_fillSvtxTrackStates = fillSvtxTrackStates;
  }

  void useActsEvaluator(bool actsEvaluator)
  {
    m_actsEvaluator = actsEvaluator;
  }
  void useActsEvaluatorSimulation(bool actsEvaluator)
  {
    m_simActsEvaluator = actsEvaluator;
    m_actsEvaluator = actsEvaluator;
  }
  void setEvaluatorName(const std::string& name) { m_evalname = name; }
  void setFieldMap(const std::string& fieldMap)
  {
    m_fieldMap = fieldMap;
  }

  void setAbsPdgHypothesis(unsigned int pHypothesis)
  {
    m_pHypothesis = pHypothesis;
  }

  void commissioning(bool com) { m_commissioning = com; }

  void useOutlierFinder(bool outlier) { m_useOutlierFinder = outlier; }
  void setOutlierFinderOutfile(const std::string& outfilename)
  {
    m_outlierFinder.outfileName(outfilename);
  }

  void SetIteration(int iter) { _n_iteration = iter; }
  void set_track_map_name(const std::string& map_name) { _track_map_name = map_name; }
  void set_svtx_seed_map_name(const std::string& map_name) { _svtx_seed_map_name = map_name; }

  void set_svtx_alignment_state_map_name(const std::string& map_name)
  {
    _svtx_alignment_state_map_name = map_name;
    m_alignStates.alignmentStateMap(map_name);
  }

  /// Set flag for pp running
  void set_pp_mode(bool ispp) { m_pp_mode = ispp; }

  void set_enable_geometric_crossing_estimate(bool flag) { m_enable_crossing_estimate = flag; }
  void set_use_clustermover(bool use) { m_use_clustermover = use; }
  void ignoreLayer(int layer) { m_ignoreLayer.insert(layer); }
  void setTrkrClusterContainerName(std::string& name) { m_clusterContainerName = name; }
  void setDirectNavigation(bool flag) { m_directNavigation = flag; }

 private:
  /// Get all the nodes
  int getNodes(PHCompositeNode* topNode);

  /// Create new nodes
  int createNodes(PHCompositeNode* topNode);

  void loopTracks(Acts::Logging::Level logLevel);

  /// Convert the acts track fit result to an svtx track
  void updateSvtxTrack(
      const std::vector<Acts::MultiTrajectoryTraits::IndexType>& tips,
      const Trajectory::IndexedParameters& paramsMap,
      const ActsTrackFittingAlgorithm::TrackContainer& tracks,
      SvtxTrack* track);

  /// Helper function to call either the regular navigation or direct
  /// navigation, depending on m_fitSiliconMMs
  ActsTrackFittingAlgorithm::TrackFitterResult fitTrack(
      const std::vector<Acts::SourceLink>& sourceLinks,
      const ActsTrackFittingAlgorithm::TrackParameters& seed,
      const ActsTrackFittingAlgorithm::GeneralFitterOptions&
          kfOptions,
      const SurfacePtrVec& surfSequence,
      const CalibratorAdapter& calibrator,
      ActsTrackFittingAlgorithm::TrackContainer& tracks);

  /// Functions to get list of sorted surfaces for direct navigation, if
  /// applicable
  SourceLinkVec getSurfaceVector(const SourceLinkVec& sourceLinks,
                                 SurfacePtrVec& surfaces) const;
  void checkSurfaceVec(SurfacePtrVec& surfaces) const;

  bool getTrackFitResult(const FitResult& fitOutput, TrackSeed* seed,
                         SvtxTrack* track,
                         const ActsTrackFittingAlgorithm::TrackContainer& tracks,
                         const ActsTrackFittingAlgorithm::MeasurementContainer& measurements);

  Acts::BoundSquareMatrix setDefaultCovariance() const;
  void printTrackSeed(const ActsTrackFittingAlgorithm::TrackParameters& seed) const;

  /// Event counter
  int m_event = 0;

  /// Options that Acts::Fitter needs to run from MakeActsGeometry
  ActsGeometry* m_tGeometry = nullptr;

  /// Configuration containing the fitting function instance
  ActsTrackFittingAlgorithm::Config m_fitCfg;

  /// TrackMap containing SvtxTracks
  alignmentTransformationContainer* m_alignmentTransformationMap = nullptr;  // added for testing purposes
  alignmentTransformationContainer* m_alignmentTransformationMapTransient = nullptr;
  std::set<Acts::GeometryIdentifier> m_transient_id_set;
  Acts::GeometryContext m_transient_geocontext;
  SvtxTrackMap* m_trackMap = nullptr;
  SvtxTrackMap* m_directedTrackMap = nullptr;
  TrkrClusterContainer* m_clusterContainer = nullptr;
  TrackSeedContainer* m_seedMap = nullptr;
  TrackSeedContainer* m_tpcSeeds = nullptr;
  TrackSeedContainer* m_siliconSeeds = nullptr;

  /// Number of acts fits that returned an error
  int m_nBadFits = 0;

  /// Boolean to use normal tracking geometry navigator or the
  /// Acts::DirectedNavigator with a list of sorted silicon+MM surfaces
  bool m_fitSiliconMMs = false;

  bool m_forceSiOnlyFit = false;

  /// requires micromegas present when fitting silicon-MM surfaces
  bool m_useMicromegas = true;

  /// A bool to update the SvtxTrackState information (or not)
  bool m_fillSvtxTrackStates = true;

  /// bool to ignore the silicon clusters in the fit
  bool m_ignoreSilicon = false;

  /// A bool to use the chi2 outlier finder in the track fitting
  bool m_useOutlierFinder = false;
  ResidualOutlierFinder m_outlierFinder;

  /// Flag for pp running
  bool m_pp_mode = false;

  bool m_directNavigation = true;

  // do we have a constant field
  bool m_ConstField{false};
  double fieldstrength{std::numeric_limits<double>::quiet_NaN()};

  // max variation of bunch crossing away from crossing_estimate
  short int max_bunch_search = 2;

  // name of TRKR_CLUSTER container
  std::string m_clusterContainerName = "TRKR_CLUSTER";

  //!@name evaluator
  //@{
  bool m_actsEvaluator = false;
  bool m_simActsEvaluator = false;
  std::unique_ptr<ActsEvaluator>
      m_evaluator = nullptr;
  std::string m_evalname = "ActsEvaluator.root";
  //@}

  //! tracks
  //  SvtxTrackMap* m_seedTracks = nullptr;

  //! tpc global position wrapper
  TpcGlobalPositionWrapper m_globalPositionWrapper;

  //! list of layers to be removed from fit
  std::set<int> m_ignoreLayer;

  bool m_use_clustermover = true;

  std::string m_fieldMap;

  int _n_iteration = 0;
  std::string _track_map_name = "SvtxTrackMap";
  std::string _svtx_seed_map_name = "SvtxTrackSeedContainer";
  std::string _svtx_alignment_state_map_name = "SvtxAlignmentStateMap";

  /// Default particle assumption to pion
  unsigned int m_pHypothesis = 211;

  SvtxAlignmentStateMap* m_alignmentStateMap = nullptr;
  ActsAlignmentStates m_alignStates;
  bool m_commissioning = false;

  bool m_enable_crossing_estimate = false;

  PHG4TpcGeomContainer* _tpccellgeo = nullptr;

  /// Variables for doing event time execution analysis
  bool m_timeAnalysis = false;
  TFile* m_timeFile = nullptr;
  TH1* h_eventTime = nullptr;
  TH2* h_fitTime = nullptr;
  TH1* h_updateTime = nullptr;
  TH1* h_stateTime = nullptr;
  TH1* h_rotTime = nullptr;

  std::vector<const Acts::Surface*> m_materialSurfaces = {};

  struct MaterialSurfaceSelector
  {
    std::vector<const Acts::Surface*> surfaces = {};

    /// @param surface is the test surface
    void operator()(const Acts::Surface* surface)
    {
      if (surface->surfaceMaterial() != nullptr)
      {
        if (std::find(surfaces.begin(), surfaces.end(), surface) ==
            surfaces.end())
        {
          surfaces.push_back(surface);
        }
      }
    }
  };
};

#endif