#include "HepMCJetTrigger.h"

#include <fun4all/SubsysReco.h>
#include <phhepmc/PHHepMCGenEvent.h>
#include <phhepmc/PHHepMCGenEventMap.h>

#include <fun4all/Fun4AllReturnCodes.h>

#include <phool/PHCompositeNode.h>
#include <phool/getClass.h>

#include <fastjet/JetDefinition.hh>

#include <HepMC/GenEvent.h>

#include <fastjet/PseudoJet.hh>
#include <string>
#include <vector>

//____________________________________________________________________________..
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
HepMCJetTrigger::HepMCJetTrigger(float trigger_thresh, int n_incom, bool up_lim, const std::string& name)
  : SubsysReco(name)
  , threshold(trigger_thresh)
  , goal_event_number(n_incom)
  , set_event_limit(up_lim)
{
}

//____________________________________________________________________________..
int HepMCJetTrigger::process_event(PHCompositeNode* topNode)
{
  // std::cout << "HepMCJetTrigger::process_event(PHCompositeNode *topNode) Processing Event" << std::endl;
  n_evts++;
  if (this->set_event_limit == true)
  {  // needed to keep all HepMC output at the same number of events
    if (n_good >= this->goal_event_number)
    {
      return Fun4AllReturnCodes::ABORTEVENT;
    }
  }
  PHHepMCGenEventMap* phg = findNode::getClass<PHHepMCGenEventMap>(topNode, "PHHepMCGenEventMap");
  if (!phg)
  {
    return Fun4AllReturnCodes::ABORTEVENT;
  }
  for (PHHepMCGenEventMap::ConstIter eventIter = phg->begin(); eventIter != phg->end(); ++eventIter)
  {
    PHHepMCGenEvent* hepev = eventIter->second;
    if (!hepev)
    {
      return Fun4AllReturnCodes::ABORTEVENT;
    }
    HepMC::GenEvent* ev = hepev->getEvent();
    if (!ev)
    {
      return Fun4AllReturnCodes::ABORTEVENT;
    }
    bool const good_event = isGoodEvent(ev);
    if (good_event)
    {
      n_good++;
    }
    if (!good_event)
    {
      return Fun4AllReturnCodes::ABORTEVENT;
    }
  }
  return Fun4AllReturnCodes::EVENT_OK;
}

bool HepMCJetTrigger::isGoodEvent(HepMC::GenEvent* e1)
{
  // this is really just the call to actually evaluate and return the filter
  if (this->threshold == 0)
  {
    return true;
  }
  std::vector<fastjet::PseudoJet> const jets = findAllJets(e1);
  int const njetsabove = jetsAboveThreshold(jets);
  if (njetsabove > 0)
  {
    return true;
  }
  return false;
}

/**
 * @brief Cluster final-state particles into anti-kt R=0.4 jets and return the resulting PseudoJets.
 *
 * Clusters particles from the provided HepMC::GenEvent into jets using the anti-kt algorithm with radius 0.4.
 * Final-state particles with PDG IDs in the range 12–18 (neutrinos / similar, by absolute value) are excluded
 * from clustering.
 *
 * @param e1 Pointer to the HepMC::GenEvent whose particles will be clustered.
 * @return std::vector<fastjet::PseudoJet> Vector of clustered jets (inclusive jets) as FastJet PseudoJet objects.
 */
std::vector<fastjet::PseudoJet> HepMCJetTrigger::findAllJets(HepMC::GenEvent* e1)
{
  // do the fast jet clustering, antikt r=-0.4
  fastjet::JetDefinition const jetdef(fastjet::antikt_algorithm, 0.4);
  std::vector<fastjet::PseudoJet> input;
  std::vector<fastjet::PseudoJet> output;
  for (HepMC::GenEvent::particle_const_iterator iter = e1->particles_begin(); iter != e1->particles_end(); ++iter)
  {
    if (!(*iter)->end_vertex() && (*iter)->status() == 1)
    {
      auto p = (*iter)->momentum();
      auto pd = std::abs((*iter)->pdg_id());
      if (pd >= 12 && pd <= 18)
      {
        continue;  // keep jet in the expected behavioro
      }
      fastjet::PseudoJet pj(p.px(), p.py(), p.pz(), p.e());
      pj.set_user_index((*iter)->barcode());
      input.push_back(pj);
    }
  }
  if (input.empty())
  {
    return input;
  }
  fastjet::ClusterSequence const js(input, jetdef);
  auto j = js.inclusive_jets();
  output.reserve(j.size());
  for (const auto& j1 : j)
  {
    output.push_back(j1);  // just keep in the corect format
  }
  return output;
}

/**
 * @brief Counts jets that exceed the configured transverse momentum threshold within the central acceptance.
 *
 * Counts how many jets in `jets` have transverse momentum greater than the module's `threshold`
 * and pseudorapidity magnitude less than or equal to 1.1.
 *
 * @param jets Vector of clustered jets to evaluate.
 * @return int Number of jets with pt greater than the threshold and |eta| <= 1.1.
 */
int HepMCJetTrigger::jetsAboveThreshold(const std::vector<fastjet::PseudoJet>& jets) const
{
  // search through for the number of identified jets above the threshold
  int n_good_jets = 0;
  for (const auto& j : jets)
  {
    float const pt = j.pt();
    if (std::abs(j.eta()) > 1.1)
    {
      continue;
    }
    if (pt > this->threshold)
    {
      n_good_jets++;
    }
  }
  return n_good_jets;
}