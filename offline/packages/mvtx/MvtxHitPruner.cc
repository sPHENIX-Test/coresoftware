/**
 * @file mvtx/MvtxHitPruner.cc
 * @author D. McGlinchey
 * @date June 2018
 * @brief Implementation of MvtxHitPruner
 */
#include "MvtxHitPruner.h"
#include "CylinderGeom_Mvtx.h"

#include <g4detectors/PHG4CylinderGeom.h>
#include <g4detectors/PHG4CylinderGeomContainer.h>

#include <trackbase/MvtxDefs.h>
#include <trackbase/TrkrDefs.h>  // for hitkey, getLayer
#include <trackbase/TrkrHitSet.h>
#include <trackbase/TrkrHitSetContainer.h>
#include <trackbase/TrkrHitv2.h>

#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/SubsysReco.h>  // for SubsysReco

#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/PHNode.h>  // for PHNode
#include <phool/PHNodeIterator.h>
#include <phool/PHObject.h>  // for PHObject
#include <phool/getClass.h>
#include <phool/phool.h>  // for PHWHERE

#include <TMatrixFfwd.h>    // for TMatrixF
#include <TMatrixT.h>       // for TMatrixT, operator*
#include <TMatrixTUtils.h>  // for TMatrixTRow
#include <TVector3.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <boost/graph/adjacency_list.hpp>
#pragma GCC diagnostic pop

#include <boost/graph/connected_components.hpp>

#include <array>
#include <cmath>
#include <cstdlib>  // for exit
#include <iostream>
#include <map>  // for multimap<>::iterator
#include <set>  // for set, set<>::iterator
#include <string>
#include <vector>  // for vector

namespace
{
  //! range adaptor to be able to use range-based for loop
  template <class T>
  class range_adaptor
  {
   public:
    /**
     * @brief Constructs a range adaptor wrapping the provided range.
     *
     * @param range Range to be adapted and iterated over by this adaptor.
     */
    explicit range_adaptor(const T& range)
      : m_range(range)
    {
    }
    /**
 * @brief Accesses the first element of the underlying range pair.
 *
 * @return const typename T::first_type& Reference to the first element (begin) of the stored range pair.
 */
const typename T::first_type& begin() { return m_range.first; }
    /**
 * @brief Access the end element of the underlying range.
 *
 * @return const typename T::second_type& Reference to the range's second (end) element; read-only access.
 */
const typename T::second_type& end() { return m_range.second; }

   private:
    T m_range;
  };
}  /**
 * @brief Construct an MvtxHitPruner with the specified module name.
 *
 * @param name Identifier used to register and reference this SubsysReco module.
 */

MvtxHitPruner::MvtxHitPruner(const std::string& name)
  : SubsysReco(name)
{
}

/**
 * @brief Perform per-run initialization for the MvtxHitPruner.
 *
 * No per-run setup is performed by this implementation.
 *
 * @return int Fun4AllReturnCodes::EVENT_OK to indicate successful initialization.
 */
int MvtxHitPruner::InitRun(PHCompositeNode* /*topNode*/)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

/**
 * @brief Consolidates MVTX hitsets across strobe values into the strobe-0 hitset and removes the non-zero strobe hitsets.
 *
 * For each MVTX hitset with a non-zero strobe, copies any hits not already present into the corresponding
 * strobe-0 (bare) hitset and then deletes the non-zero-strobe hitset. Duplicate hits (same hit key) are not copied.
 *
 * @param topNode Root of the node tree from which the "TRKR_HITSET" container is retrieved.
 * @return int `EVENT_OK` on success, `ABORTRUN` if the "TRKR_HITSET" node cannot be found.
 */
int MvtxHitPruner::process_event(PHCompositeNode* topNode)
{
  // get node containing the digitized hits
  m_hits = findNode::getClass<TrkrHitSetContainer>(topNode, "TRKR_HITSET");
  if (!m_hits)
  {
    std::cout << PHWHERE << "ERROR: Can't find node TRKR_HITSET" << std::endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }

  /*
   * We want to combine all strobe values for a given hitset
   * Start by looping over all MVTX hitsets and making a map of physical sensor
   * to hitsetkey-with-strobe
   */

  // map (bare hitset, hitset with strobe)
  std::multimap<TrkrDefs::hitsetkey, TrkrDefs::hitsetkey> hitset_multimap;

  // list of all physical sensor hitsetkeys (with strobe set to zero)
  std::set<TrkrDefs::hitsetkey> bare_hitset_set;

  const auto hitsetrange = m_hits->getHitSets(TrkrDefs::TrkrId::mvtxId);
  for (const auto& [hitsetkey, hitset] : range_adaptor(hitsetrange))
  {
    // get strobe, skip if already zero
    const int strobe = MvtxDefs::getStrobeId(hitsetkey);
    if (strobe == 0)
    {
      continue;
    }

    // get the hitsetkey value for strobe 0
    const auto bare_hitsetkey = MvtxDefs::resetStrobe(hitsetkey);
    hitset_multimap.emplace(bare_hitsetkey, hitsetkey);
    bare_hitset_set.insert(bare_hitsetkey);

    if (Verbosity())
    {
      std::cout << " found hitsetkey " << hitsetkey << " for bare_hitsetkey " << bare_hitsetkey << std::endl;
    }
  }

  // Now consolidate all hits into the hitset with strobe 0, and delete the
  // other hitsets
  //==============================================================
  for (const auto& bare_hitsetkey : bare_hitset_set)
  {
    // find matching hitset of creater
    auto* bare_hitset = (m_hits->findOrAddHitSet(bare_hitsetkey))->second;
    if (Verbosity())
    {
      std::cout
          << "MvtxHitPruner::process_event - bare_hitset " << bare_hitsetkey
          << " initially has " << bare_hitset->size() << " hits "
          << std::endl;
    }

    // get all hitsets with non-zero strobe that match the bare hitset key
    auto bare_hitsetrange = hitset_multimap.equal_range(bare_hitsetkey);
    for (const auto& [unused, hitsetkey] : range_adaptor(bare_hitsetrange))
    {
      const int strobe = MvtxDefs::getStrobeId(hitsetkey);
      if (strobe == 0)
      {
        continue;
      }

      if (Verbosity())
      {
        std::cout << "MvtxHitPruner::process_event -"
                  << " process hitsetkey " << hitsetkey
                  << " from strobe " << strobe
                  << " for bare_hitsetkey " << bare_hitsetkey
                  << std::endl;
      }

      // copy all hits to the hitset with strobe 0
      auto* hitset = m_hits->findHitSet(hitsetkey);

      if (Verbosity())
      {
        std::cout << "MvtxHitPruner::process_event - hitsetkey " << hitsetkey
                  << " has strobe " << strobe << " and has " << hitset->size()
                  << " hits,  so copy it" << std::endl;
      }

      TrkrHitSet::ConstRange hitrangei = hitset->getHits();
      for (const auto& [hitkey, old_hit] : range_adaptor(hitrangei))
      {
        if (Verbosity())
        {
          std::cout << "MvtxHitPruner::process_event - found hitkey " << hitkey << std::endl;
        }

        // if it is already there, leave it alone, this is a duplicate hit
        if (bare_hitset->getHit(hitkey))
        {
          if (Verbosity())
          {
            std::cout
                << "MvtxHitPruner::process_event - hitkey " << hitkey
                << " is already in bare hitsest, do not copy"
                << std::endl;
          }
          continue;
        }

        // otherwise copy the hit over
        if (Verbosity())
        {
          std::cout
              << "MvtxHitPruner::process_event - copying over hitkey "
              << hitkey << std::endl;
        }

        auto* new_hit = new TrkrHitv2;
        new_hit->CopyFrom(old_hit);
        bare_hitset->addHitSpecificKey(hitkey, new_hit);
      }

      // all hits are copied over to the strobe zero hitset, remove this
      // hitset
      m_hits->removeHitSet(hitsetkey);
    }
  }

  return Fun4AllReturnCodes::EVENT_OK;
}