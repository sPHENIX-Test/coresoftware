#ifndef JETBACKGROUND_DETERMINETOWERBACKGROUND_H
#define JETBACKGROUND_DETERMINETOWERBACKGROUND_H

//===========================================================
/// \file DetermineTowerBackground.h
/// \brief UE background calculator
/// \author Dennis V. Perepelitsa
//===========================================================

#include <fun4all/SubsysReco.h>

// system includes
#include <jetbase/Jet.h>
#include <string>
#include <vector>
#include <array>

// forward declarations
class PHCompositeNode;

/// \class DetermineTowerBackground
///
/// \brief UE background calculator
///
/// This module constructs dE/deta vs. eta and v2 estimates given an
/// (unsubtracted) set of calorimeter towers and possible a set of
/// exclusion jets (seeds)
///
class DetermineTowerBackground : public SubsysReco
{
 public:
  DetermineTowerBackground(const std::string &name = "DetermineTowerBackground");
  ~DetermineTowerBackground() override {}

  int InitRun(PHCompositeNode *topNode) override;
  int process_event(PHCompositeNode *topNode) override;

  /**
 * Set the name used for the background output object.
 * @param name Name to assign to the background output object.
 */
void SetBackgroundOutputName(const std::string &name) { _backgroundName = name; }
  /**
 * Configure how seed jets are handled.
 * @param seed_type Seed-type code that selects the module's seed-jet handling behavior.
 */
void SetSeedType(int seed_type) { _seed_type = seed_type; }
  /**
 * Enable or disable flow-related processing for background determination.
 * 
 * @param do_flow Integer flag controlling flow processing: 0 disables flow processing, non-zero enables it.
 */
void SetFlow(int do_flow) { _do_flow = do_flow; };
  /**
   * Enable overwriting the average calorimeter v2 with a calibration file at the given path or URL.
   * @param url Path or URL to the calibration resource to use for the average calo v2 override.
   */
  void SetOverwriteCaloV2(std::string &url)
  {
    m_overwrite_average_calo_v2 = true;
    m_overwrite_average_calo_v2_path = url;
  }
  /**
 * Set the radial distance parameter used when handling seed jets.
 *
 * @param D Radial distance in eta-phi space (R) used to define the seed-jet radius. 
 */
void SetSeedJetD(float D) { _seed_jet_D = D; };
  /**
 * Set seed jet pT threshold used to identify seed jets.
 * @param pt Threshold transverse momentum for seed jets (GeV/c).
 */
void SetSeedJetPt(float pt) { _seed_jet_pt = pt; };
  /**
 * Set the maximum constant used when evaluating seed jets.
 * @param max_const Maximum allowed constant for seed-related calculations.
 */
void SetSeedMaxConst(float max_const) { _seed_max_const = max_const; };

  void UseReweighting(bool do_reweight ) {  _do_reweight = do_reweight; }

  void set_towerNodePrefix(const std::string &prefix)
  {
    m_towerNodePrefix = prefix;
    return;
  }

 private:

  int CreateNode(PHCompositeNode *topNode);
  void FillNode(PHCompositeNode *topNode);

  int LoadCalibrations();

  std::vector<float> _CENTRALITY_V2;
  std::string m_calibName = "JET_AVERAGE_CALO_V2_SEPD_PSI2";
  bool m_overwrite_average_calo_v2{false};
  std::string m_overwrite_average_calo_v2_path;
  
  /**
 * Flag controlling flow-related processing.
 *
 * When zero, flow calculations are disabled; when non-zero, flow-related
 * processing (v2/Psi2 handling and related corrections) is performed.
 */
int _do_flow{0};
  /**
 * @brief Event or calibration v2 coefficient used for flow modulation.
 *
 * Stores the second-order azimuthal anisotropy (v2) value retrieved or computed
 * for the current event and used to modulate tower background estimations.
 * Defaults to 0.
 */
float _v2{0};
  /**
 * Event-plane angle Ψ2 for the current event, expressed in radians.
 *
 * Stored value used for flow-related background modulation when flow processing is enabled.
 */
float _Psi2{0};
  std::vector<std::vector<float> > _UE;
  int _nStrips{0};
  int _nTowers{0};

  int _HCAL_NETA{-1};
  int _HCAL_NPHI{-1};

  
  std::vector<std::vector<float> > _EMCAL_E;
  std::vector<std::vector<float> > _IHCAL_E;
  std::vector<std::vector<float> > _OHCAL_E;

  std::vector<std::vector<int> > _EMCAL_ISBAD;
  std::vector<std::vector<int> > _IHCAL_ISBAD;
  std::vector<std::vector<int> > _OHCAL_ISBAD;

  // 1-D energies vs. phi (integrated over eta strips with complete
  // phi coverage, and all layers)
  std::vector<float> _FULLCALOFLOW_PHI_E;
  std::vector<float> _FULLCALOFLOW_PHI_VAL;

  bool _do_reweight{true}; // flag to indicate if reweighting is used
  std::vector<float> _EMCAL_PHI_WEIGHTS;
  std::vector<float> _IHCAL_PHI_WEIGHTS;
  std::vector<float> _OHCAL_PHI_WEIGHTS;

  std::string _backgroundName{"TestTowerBackground"};

  int _seed_type{0};
  float _seed_jet_D{4.0};
  float _seed_max_const{3.0};
  float _seed_jet_pt{7.0};

  std::vector<float> _seed_eta;
  std::vector<float> _seed_phi;

  Jet::PROPERTY _index_SeedD{};
  Jet::PROPERTY _index_SeedItr{};

  bool _is_flow_failure{false};
  bool _reweight_failed{false};

  std::string m_towerNodePrefix{"TOWERINFO_CALIB"};
  std::string EMTowerName;
  std::string IHTowerName;
  std::string OHTowerName;
};

#endif