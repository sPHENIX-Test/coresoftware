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
 * Set the background output node name used by the module.
 * @param name Background output name (node key) to use when creating or locating the background data node.
 */
void SetBackgroundOutputName(const std::string &name) { _backgroundName = name; }
  /**
 * Set the seed type that selects the seed-finding strategy for exclusion seeds.
 * @param seed_type Integer selector identifying which seed-finding strategy to use.
 */
void SetSeedType(int seed_type) { _seed_type = seed_type; }
  /**
 * Enable or disable flow processing.
 * @param do_flow Non-zero to enable flow, zero to disable.
 */
void SetFlow(int do_flow) { _do_flow = do_flow; };
  /**
   * Enable overwriting the average calorimeter v2 and set the source path.
   * @param url Filesystem or resource path pointing to the replacement average calo v2 data.
   */
  void SetOverwriteCaloV2(std::string &url)
  {
    m_overwrite_average_calo_v2 = true;
    m_overwrite_average_calo_v2_path = url;
  }
  /**
 * Set the seed-jet distance parameter used to select seed jets.
 * @param D Distance parameter R for seed-jet clustering in eta-phi space.
 */
void SetSeedJetD(float D) { _seed_jet_D = D; };
  /**
 * Set the seed-jet transverse-momentum threshold.
 * 
 * @param pt Transverse momentum threshold for seed jets in GeV/c.
 */
void SetSeedJetPt(float pt) { _seed_jet_pt = pt; };
  /**
 * Set the maximum constant used when evaluating seed jets.
 * @param max_const Maximum constant applied to seed selection.
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
 * Controls whether flow modulation is applied to background calculations.
 *
 * When non-zero, flow (v2/Psi2) corrections are applied to tower/background processing;
 * when zero, flow corrections are not applied.
 */
int _do_flow{0};
  /**
 * @brief Event or bin-level second-order azimuthal anisotropy coefficient (v2).
 *
 * Stores the estimated v2 value used for flow-modulated background calculations.
 */
float _v2{0};
  /**
 * Event-plane angle used for v2 calculations.
 *
 * Stored in radians for the current event; populated when flow estimation is performed.
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