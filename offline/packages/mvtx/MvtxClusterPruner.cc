/**
 * @file mvtx/MvtxClusterPruner.cc
 * @author Hugo Pereira Da Costa <hugo.pereira-da-costa@lanl.gov>
 * @date May 2025
 * @brief Implementation of MvtxClusterPruner
 */

#include "MvtxClusterPruner.h"

#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/getClass.h>

#include <trackbase/MvtxDefs.h>
#include <trackbase/TrkrCluster.h>
#include <trackbase/TrkrClusterContainer.h>
#include <trackbase/TrkrClusterHitAssoc.h>

#include <algorithm>
#include <set>

namespace
{
  //! range adaptor to be able to use range-based for loop
  template <class T>
  class range_adaptor
  {
   public:
    /**
     * @brief Construct an adaptor that exposes a pair-like range as an iterable for range-based for loops.
     *
     * @param range Pair-like range (e.g., a std::pair of iterators or any object providing begin/end) to be copied into the adaptor for iteration.
     */
    explicit range_adaptor(const T& range)
      : m_range(range)
    {
    }
    /**
 * @brief Accesses the beginning element of the wrapped pair-like range.
 *
 * @return Reference to the first element of the stored pair representing the range start.
 */
const typename T::first_type& begin() { return m_range.first; }
    /**
 * @brief Access the end of the underlying pair-like range.
 *
 * @return const typename T::second_type& Reference to the range's end element (the pair's second).
 */
const typename T::second_type& end() { return m_range.second; }

   private:
    T m_range;
  };

  /**
   * @brief Prints MVTX cluster metadata to standard output.
   *
   * When `cluster` is non-null, prints the cluster key, local position (X,Y),
   * size, layer, stave, chip, strobe, and index. When `cluster` is null, prints
   * the cluster key, layer, stave, chip, strobe, and index.
   *
   * @param ckey Identifier for the MVTX cluster.
   * @param cluster Pointer to the cluster; may be null to indicate absence of
   *                cluster object (in which case position and size are omitted).
   */
  void print_cluster_information(TrkrDefs::cluskey ckey, TrkrCluster* cluster)
  {
    if (cluster)
    {
      std::cout << " MVTX cluster: " << ckey
                << " position: (" << cluster->getLocalX() << ", " << cluster->getLocalY() << ")"
                << " size: " << (int) cluster->getSize()
                << " layer: " << (int) TrkrDefs::getLayer(ckey)
                << " stave: " << (int) MvtxDefs::getStaveId(ckey)
                << " chip: " << (int) MvtxDefs::getChipId(ckey)
                << " strobe: " << MvtxDefs::getStrobeId(ckey)
                << " index: " << (int) TrkrDefs::getClusIndex(ckey)
                << std::endl;
    }
    else
    {
      std::cout << " MVTX cluster: " << ckey
                << " layer: " << (int) TrkrDefs::getLayer(ckey)
                << " stave: " << (int) MvtxDefs::getStaveId(ckey)
                << " chip: " << (int) MvtxDefs::getChipId(ckey)
                << " strobe: " << MvtxDefs::getStrobeId(ckey)
                << " index: " << (int) TrkrDefs::getClusIndex(ckey)
                << std::endl;
    }
  }

  using hitkeyset_t = std::set<TrkrDefs::hitkey>;
  using clustermap_t = std::map<TrkrDefs::cluskey, hitkeyset_t>;

}  // namespace

/**
 * @brief Constructs the MvtxClusterPruner module.
 *
 * Initializes the SubsysReco base with the given module name.
 *
 * @param name Name used to identify and register this SubsysReco instance.
 */
MvtxClusterPruner::MvtxClusterPruner(const std::string& name)
  : SubsysReco(name)
{
}

/**
 * @brief Log the current strict-matching setting and perform no additional initialization.
 *
 * Prints the value of m_use_strict_matching to standard output and completes initialization.
 *
 * @return int Fun4AllReturnCodes::EVENT_OK to indicate successful initialization.
 */
int MvtxClusterPruner::InitRun(PHCompositeNode* /*topNode*/)
{
  std::cout << "MvtxClusterPruner::InitRun - m_use_strict_matching: " << m_use_strict_matching << std::endl;
  return Fun4AllReturnCodes::EVENT_OK;
}

/**
 * @brief Prune redundant MVTX clusters across consecutive strobe keys.
 *
 * Compares clusters from a given MVTX hitset to clusters in the next strobe and removes redundant clusters:
 * - If strict matching is enabled, removes clusters whose associated hit sets are identical.
 * - If strict matching is disabled, removes clusters whose associated hit set is a subset of another cluster's hit set.
 *
 * @param topNode The root node of the PHENIX/Acts node tree used to access TRKR_CLUSTER and TRKR_CLUSTERHITASSOC.
 * @return Fun4All return code: `Fun4AllReturnCodes::EVENT_OK` on success (processing completed or skipped if required nodes are absent).
 */
int MvtxClusterPruner::process_event(PHCompositeNode* topNode)
{
  // load relevant nodes
  auto* trkrclusters = findNode::getClass<TrkrClusterContainer>(topNode, "TRKR_CLUSTER");
  if (!trkrclusters)
  {
    std::cout << "MvtxClusterPruner::process_event - TRKR_CLUSTER not found. Doing nothing" << std::endl;
    return Fun4AllReturnCodes::EVENT_OK;
  }

  auto* clusterhitassoc = findNode::getClass<TrkrClusterHitAssoc>(topNode, "TRKR_CLUSTERHITASSOC");
  if (!clusterhitassoc)
  {
    std::cout << "MvtxClusterPruner::process_event - TRKR_CLUSTERHITASSOC not found. Doing nothing" << std::endl;
    return Fun4AllReturnCodes::EVENT_OK;
  }

  // lambda method to create map of cluster keys and associated hits
  auto get_cluster_map = [trkrclusters, clusterhitassoc](TrkrDefs::hitsetkey key)
  {
    clustermap_t out;

    // get all clusters for this hitsetkey
    const auto cluster_range = trkrclusters->getClusters(key);
    for (const auto& [ckey, cluster] : range_adaptor(cluster_range))
    {
      // get associated hits
      const auto& hit_range = clusterhitassoc->getHits(ckey);
      hitkeyset_t hitkeys;
      std::transform(hit_range.first, hit_range.second, std::inserter(hitkeys, hitkeys.end()),
                     [](const TrkrClusterHitAssoc::Map::value_type& pair)
                     { return pair.second; });
      out.emplace(ckey, std::move(hitkeys));
    }
    return out;
  };

  // loop over MVTX hitset keys
  const auto hitsetkeys = trkrclusters->getHitSetKeys(TrkrDefs::mvtxId);
  for (const auto& hitsetkey : hitsetkeys)
  {
    // get layer, stave, chip and current strobe
    const auto layer = TrkrDefs::getLayer(hitsetkey);
    const auto stave = MvtxDefs::getStaveId(hitsetkey);
    const auto chip = MvtxDefs::getChipId(hitsetkey);
    const auto current_strobe = MvtxDefs::getStrobeId(hitsetkey);

    // get clusters for this hitsetkey
    const auto cluster_map1 = get_cluster_map(hitsetkey);

    // get clusters for the next strobe
    int next_strobe = current_strobe + 1;
    const auto hitsetkey_next_strobe = MvtxDefs::genHitSetKey(layer, stave, chip, next_strobe);
    const auto clusterk_map2 = get_cluster_map(hitsetkey_next_strobe);

    // loop over clusters from first range
    for (auto [ckey1, hitkeys1] : cluster_map1)
    {
      // increment counter
      ++m_cluster_counter_total;

      // get correcponding cluser
      auto* cluster1 = Verbosity() ? trkrclusters->findCluster(ckey1) : nullptr;

      // loop over clusters from second range
      for (auto [ckey2, hitkeys2] : clusterk_map2)
      {
        auto* cluster2 = Verbosity() ? trkrclusters->findCluster(ckey2) : nullptr;

        if (m_use_strict_matching)
        {
          // see if hitsets are identical
          if (hitkeys1 == hitkeys2)
          {
            // increment counter
            ++m_cluster_counter_deleted;

            if (Verbosity())
            {
              std::cout << "Removing cluster ";
              print_cluster_information(ckey2, cluster2);

              std::cout << "Keeping  cluster ";
              print_cluster_information(ckey1, cluster1);
            }

            // always remove second cluster
            trkrclusters->removeCluster(ckey2);
            break;
          }
        }
        else
        {
          // make sure first set is larger than second
          const bool swapped = hitkeys2.size() > hitkeys1.size();
          if (swapped)
          {
            std::swap(hitkeys2, hitkeys1);
          }

          // see if hitkeys2 is a subset of hitkeys1
          if (std::includes(hitkeys1.begin(), hitkeys1.end(), hitkeys2.begin(), hitkeys2.end()))
          {
            // increment counter
            ++m_cluster_counter_deleted;

            if (swapped)
            {
              if (Verbosity())
              {
                std::cout << "Removing cluster ";
                print_cluster_information(ckey1, cluster1);

                std::cout << "Keeping  cluster ";
                print_cluster_information(ckey2, cluster2);
              }

              // remove first cluster
              trkrclusters->removeCluster(ckey1);
              break;
            }
            if (Verbosity())
            {
              std::cout << "Removing cluster ";
              print_cluster_information(ckey2, cluster2);

              std::cout << "Keeping  cluster ";
              print_cluster_information(ckey1, cluster1);
            }

            // remove second cluster
            trkrclusters->removeCluster(ckey2);
          }

        }  // strict matching

      }  // second cluster loop
    }  // first cluster loop
  }  // hitsetkey loop

  return Fun4AllReturnCodes::EVENT_OK;
}

/**
 * @brief Logs final cluster pruning statistics.
 *
 * Prints the total number of cluster comparisons, the number of clusters removed,
 * and the fraction removed, then completes module finalization.
 *
 * @return int Fun4AllReturnCodes::EVENT_OK on success.
 */
int MvtxClusterPruner::End(PHCompositeNode* /*topNode*/)
{
  std::cout << "MvtxClusterPruner::End -"
            << " m_cluster_counter_total: " << m_cluster_counter_total
            << std::endl;

  std::cout << "MvtxClusterPruner::End -"
            << " m_cluster_counter_deleted: " << m_cluster_counter_deleted
            << " fraction: " << double(m_cluster_counter_deleted) / m_cluster_counter_total
            << std::endl;

  return Fun4AllReturnCodes::EVENT_OK;
}