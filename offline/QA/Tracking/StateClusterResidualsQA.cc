#include "StateClusterResidualsQA.h"

#include <trackbase/InttDefs.h>
#include <trackbase/MvtxDefs.h>
#include <trackbase/TpcDefs.h>
#include <trackbase/TrkrCluster.h>
#include <trackbase/TrkrClusterContainer.h>
#include <trackbase/ActsGeometry.h>

#include <trackbase_historic/SvtxAlignmentState.h>
#include <trackbase_historic/SvtxTrack.h>
#include <trackbase_historic/SvtxTrackMap.h>
#include <trackbase_historic/SvtxTrackState.h>

#include <qautils/QAHistManagerDef.h>
#include <qautils/QAUtil.h>

#include <fun4all/Fun4AllHistoManager.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/SubsysReco.h>

#include <phool/PHCompositeNode.h>
#include <phool/getClass.h>

#include <Rtypes.h>
#include <TH1F.h>

#include <format>

namespace
{
  template <typename T>
  /**
 * @brief Computes the square of a value.
 *
 * @tparam T Type of the value; must support multiplication.
 * @param t Value to be squared.
 * @return T The product of `t` with itself.
 */
inline T square (T const& t) { return t * t; }

  template <class T>
  class range_adaptor
  {
   public:
    /**
     * @brief Construct a range adaptor from a pair of iterators.
     *
     * Stores the provided begin and end iterators/keys for later use by begin() and end().
     *
     * @param begin Iterator or key pointing to the start of the range.
     * @param end Iterator or key pointing one past the end of the range.
     */
    explicit range_adaptor(
        T const& begin,
        T const& end)
      : m_begin(begin)
      , m_end(end)
    {
    }
    /**
 * @brief Accesses the stored begin iterator.
 *
 * @return T const& A const reference to the stored begin iterator or key.
 */
T const& begin() { return m_begin; }
    /**
 * @brief Access the stored end iterator or key.
 *
 * @return T const& Reference to the stored end iterator or key.
 */
T const& end() { return m_end; }

   private:
    T m_begin;
    T m_end;
  };
}  /**
 * @brief Construct a StateClusterResidualsQA module with the specified name.
 *
 * The provided name is used to identify this QA module instance within the
 * Fun4All/SubsysReco framework.
 *
 * @param name Module name used for identification and bookkeeping.
 */

StateClusterResidualsQA::StateClusterResidualsQA(const std::string& name)
  : SubsysReco(name)
{
}

/**
 * @brief Initialize the QA module for a run and bind required resources.
 *
 * Creates histograms and resolves required nodes from the provided top-level
 * node tree. Populates the internal histogram vectors with handles retrieved
 * from the QA histogram manager.
 *
 * @param top_node Top-level PHCompositeNode used to look up detector, cluster,
 * and geometry nodes.
 * @return int `EVENT_OK` on success; `ABORTRUN` if a required node or the QA
 * histogram manager could not be retrieved.
 */
int StateClusterResidualsQA::InitRun(
    PHCompositeNode* top_node)
{
  createHistos();

  // F4A will not actually ABORTRUN unless that return code is issued here
  auto* track_map = findNode::getClass<SvtxTrackMap>(top_node, m_track_map_node_name);
  if (!track_map)
  {
    std::cout
        << PHWHERE << "\n"
        << "\tCould not get track map:\n"
        << "\t\"" << m_track_map_node_name << "\"\n"
        << "\tAborting\n"
        << std::endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }

  auto* cluster_map = findNode::getClass<TrkrClusterContainer>(top_node, m_clusterContainerName);
  if (!cluster_map)
  {
    std::cout
        << PHWHERE << "\n"
        << "\tCould not get cluster map:\n"
        << "\t\"" << m_clusterContainerName << "\"\n"
        << "\tAborting\n"
        << std::endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }
  
  auto *geometry = findNode::getClass<ActsGeometry>(top_node, "ActsGeometry");
  if (!geometry)
  {
    std::cout
        << PHWHERE << "\n"
        << "\tCould not get ActsGeometry:\n"
        << "\t\"" << "ActsGeometry" << "\"\n"
        << "\tAborting\n"
        << std::endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }

  auto* hm = QAHistManagerDef::getHistoManager();
  if (!hm)
  {
    std::cout
        << PHWHERE << "\n"
        << "\tCould not get QAHistManager\n"
        << "\tAborting\n"
        << std::endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }

  for (const auto& cfg : m_pending)
  {
    m_histograms_x.push_back(dynamic_cast<TH1 *>(hm->getHisto(std::string(cfg.name + "_x"))));
    m_histograms_y.push_back(dynamic_cast<TH1 *>(hm->getHisto(std::string(cfg.name + "_y"))));
    m_histograms_z.push_back(dynamic_cast<TH1 *>(hm->getHisto(std::string(cfg.name + "_z"))));
  }
  
  return Fun4AllReturnCodes::EVENT_OK;
}

/**
 * @brief Process one event: fill state–cluster residual histograms for tracks matching configured selections.
 *
 * Iterates all tracks in the SvtxTrackMap, counts per-detector clusters per track, applies per-configuration
 * selection cuts (charge, per-detector cluster count ranges, phi/eta/pt ranges), and for tracks that pass,
 * computes residuals between track state positions and corresponding cluster global positions (state - cluster)
 * in x, y, and z and fills the configured histograms. The initial vertex state (path_length == 0) is skipped.
 *
 * @param top_node Top-level node of the Fun4All node tree from which required data (track map, cluster container,
 *                 geometry) are retrieved.
 * @return int Fun4All return code: `EVENT_OK` on success.
 */
int StateClusterResidualsQA::process_event(PHCompositeNode* top_node)
{
  auto* track_map = findNode::getClass<SvtxTrackMap>(top_node, m_track_map_node_name);
  auto *cluster_map = findNode::getClass<TrkrClusterContainer>(top_node, m_clusterContainerName);
  auto *geometry = findNode::getClass<ActsGeometry>(top_node, "ActsGeometry");

  for (auto const& [idkey, track] : *track_map)
  {
    if (!track)
    {
      continue;
    }

    // count states
    std::map<TrkrDefs::TrkrId, int> counters = {
        {TrkrDefs::mvtxId, 0},
        {TrkrDefs::inttId, 0},
        {TrkrDefs::tpcId, 0},
        {TrkrDefs::micromegasId, 0},
    };

    for (auto const& [path_length, state] : range_adaptor(track->begin_states(), track->end_states()))
    {
      // There is an additional state representing the vertex at the beginning of the map,
      // but getTrkrId will return 0 for its corresponding cluster
      // Identify it as having path_length identically equal to 0
      if (path_length == 0) { continue; }

      auto trkr_id = static_cast<TrkrDefs::TrkrId>(TrkrDefs::getTrkrId(state->get_cluskey()));
      auto itr = counters.find(trkr_id);
      if (itr == counters.end()) { continue; }
      ++itr->second;
    }

    float track_eta = track->get_eta();
    float track_phi = track->get_phi();
    float track_pt = track->get_pt();
    int h = 0;
    for (const auto& cfg : m_pending)
    {
      if (cfg.charge != 0)
      {
        if ((cfg.charge < 0) && track->get_positive_charge())
        {
          continue;
        }
        if ((cfg.charge > 0) && !(track->get_positive_charge()))
        {
          continue;
        }
      }
      if (cfg.min_mvtx_clusters <= counters[TrkrDefs::mvtxId] && cfg.max_mvtx_clusters >= counters[TrkrDefs::mvtxId]
          && cfg.min_intt_clusters <= counters[TrkrDefs::inttId] && cfg.max_intt_clusters >= counters[TrkrDefs::inttId]
          && cfg.min_tpc_clusters <= counters[TrkrDefs::tpcId] && cfg.max_tpc_clusters >= counters[TrkrDefs::tpcId]
          && cfg.phi_min <= track_phi && cfg.phi_max >= track_phi
          && cfg.eta_min <= track_eta && cfg.eta_max >= track_eta
          && cfg.pt_min <= track_pt && cfg.pt_max >= track_pt)
      {
        for (auto const& [path_length, state] : range_adaptor(track->begin_states(), track->end_states()))
        {
          if (path_length == 0) { continue; }

          auto *cluster = cluster_map->findCluster(state->get_cluskey());
          float state_x = state->get_x();
          float state_y = state->get_y();
          float state_z = state->get_z();
          Acts::Vector3 glob = geometry->getGlobalPosition(state->get_cluskey(), cluster);
          float cluster_x = glob.x();
          float cluster_y = glob.y();
          float cluster_z = glob.z();
          if (cluster)
          {
            m_histograms_x[h]->Fill(state_x - cluster_x);
            m_histograms_y[h]->Fill(state_y - cluster_y);
            m_histograms_z[h]->Fill(state_z - cluster_z);
          }
        }
      } 
      ++h;
    }
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

/**
 * @brief Create and register residual histograms for each pending configuration.
 *
 * For each entry in m_pending this creates three TH1F histograms named
 * "<cfg.name>_x", "<cfg.name>_y", and "<cfg.name>_z" for the State-Cluster
 * residuals in X, Y, and Z (units: cm). Each histogram is initialized with
 * m_nBins and the corresponding axis range (m_xrange, m_yrange, m_zrange) and
 * styled with blue marker and line color before being registered with the QA
 * histogram manager.
 */
void StateClusterResidualsQA::createHistos()
{
  auto *hm = QAHistManagerDef::getHistoManager();
  assert(hm);

  for (const auto& cfg : m_pending)
  {
    TH1F* h_new_x = new TH1F(
        (cfg.name + "_x").c_str(),
        ";State-Cluster X Residual [cm];Entries",
        m_nBins, m_xrange.first, m_xrange.second);
    h_new_x->SetMarkerColor(kBlue);
    h_new_x->SetLineColor(kBlue);
    hm->registerHisto(h_new_x);
    TH1F* h_new_y = new TH1F(
        (cfg.name + "_y").c_str(),
        ";State-Cluster Y Residual [cm];Entries",
        m_nBins, m_yrange.first, m_yrange.second);
    h_new_y->SetMarkerColor(kBlue);
    h_new_y->SetLineColor(kBlue);
    hm->registerHisto(h_new_y);
    TH1F* h_new_z = new TH1F(
        (cfg.name + "_z").c_str(),
        ";State-Cluster Z Residual [cm];Entries",
        m_nBins, m_zrange.first, m_zrange.second);
    h_new_z->SetMarkerColor(kBlue);
    h_new_z->SetLineColor(kBlue);
    hm->registerHisto(h_new_z);
  }
}

/**
 * @brief Finalize the QA module at the end of a run.
 *
 * Asserts that the QA histogram manager is available.
 *
 * @return Fun4AllReturnCodes::EVENT_OK on success.
 */
int StateClusterResidualsQA::EndRun(const int /*unused*/)
{
  auto *hm = QAHistManagerDef::getHistoManager();
  assert(hm);

  return Fun4AllReturnCodes::EVENT_OK;
}