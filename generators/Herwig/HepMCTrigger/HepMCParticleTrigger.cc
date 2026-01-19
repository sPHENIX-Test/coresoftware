#include "HepMCParticleTrigger.h"

#include <fun4all/SubsysReco.h>
#include <phhepmc/PHHepMCGenEvent.h>
#include <phhepmc/PHHepMCGenEventMap.h>

#include <fun4all/Fun4AllReturnCodes.h>

#include <phool/PHCompositeNode.h>
#include <phool/getClass.h>

#include <fastjet/JetDefinition.hh>

#include <HepMC/GenEvent.h>

#include <fastjet/PseudoJet.hh>
#include <map>
#include <string>
#include <vector>
//____________________________________________________________________________..
/**
 * @brief Construct a HepMCParticleTrigger with configured thresholds and limits.
 *
 * Initializes trigger parameters, default kinematic cut values, and control flags;
 * optionally enables a Pt lower bound when a nonzero threshold is provided.
 *
 * @param trigger_thresh If nonzero, sets the Pt lower cut to this value and enables the Pt low cut.
 * @param n_incom Target number of accepted events used when enforcing an event limit.
 * @param up_lim If true, processing will stop once the target number of accepted events is reached.
 * @param name Module name passed to the SubsysReco base class.
 */
HepMCParticleTrigger::HepMCParticleTrigger(float trigger_thresh, int n_incom, bool up_lim, const std::string& name)
  : SubsysReco(name)
  , threshold(trigger_thresh)
  , goal_event_number(n_incom)
  , set_event_limit(up_lim)
  , _theEtaHigh(1.1)
  , _theEtaLow(-1.1)
  , _thePtHigh(999.9)
  , _thePtLow(0)
  , _thePHigh(999.9)
  , _thePLow(-999.9)
  , _thePzHigh(999.9)
  , _thePzLow(-999.9)
  ,

    _doEtaHighCut(true)
  , _doEtaLowCut(true)
  , _doBothEtaCut(true)
  ,

  _doAbsEtaHighCut(false)
  , _doAbsEtaLowCut(false)
  , _doBothAbsEtaCut(false)
  ,

  _doPtHighCut(false)
  , _doPtLowCut(false)
  , _doBothPtCut(false)
  ,
    _doPHighCut(false)
  , _doPLowCut(false)
  , _doBothPCut(false)
  ,

  _doPzHighCut(false)
  , _doPzLowCut(false)
  , _doBothPzCut(false) 
{
  if (threshold != 0)
  {
    _doPtLowCut = true;
    _thePtLow = threshold;
  }
}

/**
 * @brief Process a single PHCompositeNode event and decide whether it passes the configured HepMC particle trigger.
 *
 * Increments the processed-event counter and, if a configured event limit is reached, aborts further processing.
 * Retrieves the PHHepMCGenEventMap from the node tree, validates each contained HepMC::GenEvent, and accepts the event only if every GenEvent satisfies isGoodEvent.
 * When accepted, increments the accepted-event counter.
 *
 * @param topNode Root node of the Fun4All node tree containing the "PHHepMCGenEventMap".
 * @return int Fun4AllReturnCodes::EVENT_OK if the event is accepted; Fun4AllReturnCodes::ABORTEVENT if the node map or any GenEvent is missing, if the event fails the trigger, or if the configured accepted-event limit has been reached.
 */
int HepMCParticleTrigger::process_event(PHCompositeNode* topNode)
{
  // std::cout << "HepMCParticleTrigger::process_event(PHCompositeNode *topNode) Processing Event" << std::endl;
  n_evts++;
  if (this->set_event_limit == true)
  {  // needed to keep all HepMC output at the same number of events
    if (n_good >= this->goal_event_number)
    {
      return Fun4AllReturnCodes::ABORTEVENT;
    }
  }
  bool good_event{false};
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
    good_event = isGoodEvent(ev);
    if (!good_event)
    {
      return Fun4AllReturnCodes::ABORTEVENT;
    }
  }
  if (good_event)
  {
    n_good++;
  }
  return Fun4AllReturnCodes::EVENT_OK;
}
/**
 * @brief Adds a particle PDG ID to the list of trigger particles.
 *
 * @param particlePid PDG identifier to require for the trigger (use negative values for antiparticles).
 */
void HepMCParticleTrigger::AddParticle(int particlePid)
{
  _theParticles.push_back(particlePid);
  return;
}
/**
 * @brief Appends a collection of PDG particle IDs to the internal trigger list.
 *
 * @param particles Vector of PDG IDs to add; each ID is appended in order and duplicates are preserved.
 */
void HepMCParticleTrigger::AddParticles(const std::vector<int>& particles)
{
  for (auto p : particles)
  {
    _theParticles.push_back(p);
  }
  return;
}

/**
 * @brief Sets the upper transverse momentum (pT) threshold and enables the high-pT cut.
 *
 * If a low-pT cut is already enabled, also enables the combined pT-range check.
 *
 * @param pt Upper pT threshold to apply.
 */
void HepMCParticleTrigger::SetPtHigh(double pt)
{
  _thePtHigh = pt;
  _doPtHighCut = true;
  if (_doPtLowCut)
  {
    _doBothPtCut = true;
  }
  return;
}
/**
 * @brief Set the lower bound for the transverse momentum (pt) cut and enable the pt-low selection.
 *
 * Enables the low-pt cut using the provided threshold and, if the high-pt cut is already enabled,
 * also enables the combined (both low and high) pt-range cut.
 *
 * @param pt Lower bound for transverse momentum (pt) in the same units used by event particles.
 */
void HepMCParticleTrigger::SetPtLow(double pt)
{
  _thePtLow = pt;
  _doPtLowCut = true;
  if (_doPtHighCut)
  {
    _doBothPtCut = true;
  }
  return;
}
/**
 * @brief Enable a combined transverse momentum (pT) range cut and set its bounds.
 *
 * Sets the upper and lower pT thresholds and enables the high, low, and combined pT cuts.
 *
 * @param ptHigh Upper bound on transverse momentum (pT).
 * @param ptLow Lower bound on transverse momentum (pT).
 */
void HepMCParticleTrigger::SetPtHighLow(double ptHigh, double ptLow)
{
  _thePtHigh = ptHigh;
  _doPtHighCut = true;
  _thePtLow = ptLow;
  _doPtLowCut = true;
  _doBothPtCut = true;
  return;
}
/**
 * @brief Set the upper bound for the total momentum cut and enable it.
 *
 * Enables the momentum high cut at the supplied value. If a low-momentum cut is already enabled,
 * also enables the combined (high+low) momentum-range cut.
 *
 * @param pt Upper momentum threshold (same units as input momenta, e.g., GeV/c).
 */
void HepMCParticleTrigger::SetPHigh(double pt)
{
  _thePHigh = pt;
  _doPHighCut = true;
  if (_doPLowCut)
  {
    _doBothPCut = true;
  }
  return;
}
/**
 * @brief Set the lower bound for the total momentum (P) cut and enable the low-P cut.
 *
 * Enables the P low cut using the provided value; if a P high cut is already enabled,
 * also enables the combined P high-low range check.
 *
 * @param pt Lower bound for the particle momentum P.
 */
void HepMCParticleTrigger::SetPLow(double pt)
{
  _thePLow = pt;
  _doPLowCut = true;
  if (_doPHighCut)
  {
    _doBothPCut = true;
  }
  return;
}
/**
 * @brief Set both minimum and maximum total-momentum (P) cuts and enable them.
 *
 * Enables the low and high total-momentum cuts and marks that both-P range should be applied.
 *
 * @param ptHigh Upper bound for total momentum (P).
 * @param ptLow Lower bound for total momentum (P).
 */
void HepMCParticleTrigger::SetPHighLow(double ptHigh, double ptLow)
{
  _thePHigh = ptHigh;
  _doPHighCut = true;
  _thePLow = ptLow;
  _doPLowCut = true;
  _doBothPCut = true;
  return;
}
/**
 * @brief Set the upper bound for the Pz cut and enable the high-Pz cut.
 *
 * Configures the trigger to reject particles with Pz greater than the given value by
 * setting the internal high-Pz threshold and enabling the high-Pz cut. If a low-Pz
 * cut is already enabled, also enable the combined (high-and-low) Pz range check.
 *
 * @param pt Upper Pz threshold (same units as particle Pz).
 */
void HepMCParticleTrigger::SetPzHigh(double pt)
{
  _thePzHigh = pt;
  _doPzHighCut = true;
  if (_doPzLowCut)
  {
    _doBothPzCut = true;
  }
  return;
}
/**
 * @brief Set the lower bound for the longitudinal momentum (Pz) cut.
 *
 * Sets the minimum Pz value (_thePzLow) used to filter particles and enables the low-Pz cut flag.
 * If a high-Pz cut is already enabled, also enables the combined Pz-range flag.
 *
 * @param pt Lower bound for Pz. 
 */
void HepMCParticleTrigger::SetPzLow(double pt)
{
  _thePzLow = pt;
  _doPzLowCut = true;
  if (_doPzHighCut)
  {
    _doBothPzCut = true;
  }
  return;
}
/**
 * @brief Configure both upper and lower limits for the longitudinal momentum (Pz) cut.
 *
 * Enables the Pz high and low cuts and sets their threshold values so that subsequent
 * particle selection uses the configured Pz range.
 *
 * @param ptHigh Upper bound for Pz.
 * @param ptLow Lower bound for Pz.
 */
void HepMCParticleTrigger::SetPzHighLow(double ptHigh, double ptLow)
{
  _thePzHigh = ptHigh;
  _doPzHighCut = true;
  _thePzLow = ptLow;
  _doPzLowCut = true;
  _doBothPzCut = true;
  return;
}
/**
 * @brief Set the upper eta threshold and enable the eta high cut.
 *
 * Sets the maximum allowed pseudorapidity (eta) for the trigger and enables the
 * high-eta cut. If a low-eta cut is already enabled, also enables the combined
 * eta-range (both) cut.
 *
 * @param pt Upper eta limit to apply for the high-eta cut.
 */
void HepMCParticleTrigger::SetEtaHigh(double pt)
{
  _theEtaHigh = pt;
  _doEtaHighCut = true;
  if (_doEtaLowCut)
  {
    _doBothEtaCut = true;
  }
  return;
}
/**
 * @brief Set the lower bound for the pseudorapidity (eta) selection and enable the corresponding cut.
 *
 * Enables the eta-low cut using the provided value. If an eta-high cut is already enabled, also enables the combined
 * eta-range cut.
 *
 * @param pt Lower bound for eta selection (inclusive).
 */
void HepMCParticleTrigger::SetEtaLow(double pt)
{
  _theEtaLow = pt;
  _doEtaLowCut = true;
  if (_doEtaHighCut)
  {
    _doBothEtaCut = true;
  }
  return;
}
/**
 * @brief Set both upper and lower pseudorapidity (eta) thresholds and enable their cuts.
 *
 * Sets the high and low eta limits used by the trigger and enables the individual
 * high/low eta cut flags as well as the combined eta-range flag.
 *
 * @param ptHigh Upper (maximum) eta threshold.
 * @param ptLow Lower (minimum) eta threshold.
 */
void HepMCParticleTrigger::SetEtaHighLow(double ptHigh, double ptLow)
{
  _theEtaHigh = ptHigh;
  _doEtaHighCut = true;
  _theEtaLow = ptLow;
  _doEtaLowCut = true;
  _doBothEtaCut = true;
  return;
}
/**
 * @brief Set the upper bound for the absolute pseudorapidity (|eta|) cut.
 *
 * Enables the absolute-eta high cut and, if an absolute-eta low cut is already enabled,
 * enables the combined absolute-eta range check.
 *
 * @param pt Upper threshold for absolute pseudorapidity (|eta|).
 */
void HepMCParticleTrigger::SetAbsEtaHigh(double pt)
{
  _theEtaHigh = pt;
  _doAbsEtaHighCut = true;
  if (_doAbsEtaLowCut)
  {
    _doBothAbsEtaCut = true;
  }
  return;
}
/**
 * @brief Configure the lower bound for the absolute pseudorapidity (|eta|) cut.
 *
 * Sets the lower |eta| threshold and enables the absolute-eta low cut. If an absolute-eta
 * high cut is already enabled, also enables the combined absolute-eta range check.
 *
 * @param pt Lower threshold for |eta|.
 */
void HepMCParticleTrigger::SetAbsEtaLow(double pt)
{
  _theEtaLow = pt;
  _doAbsEtaLowCut = true;
  if (_doAbsEtaHighCut)
  {
    _doBothAbsEtaCut = true;
  }
  return;
}
/**
 * @brief Configure absolute pseudorapidity (|eta|) high and low cut thresholds.
 *
 * Enables and sets the absolute-eta upper and lower bounds used when filtering particles.
 *
 * @param ptHigh Upper bound for absolute pseudorapidity (|eta|).
 * @param ptLow Lower bound for absolute pseudorapidity (|eta|).
 */
void HepMCParticleTrigger::SetAbsEtaHighLow(double ptHigh, double ptLow)
{
  _theEtaHigh = ptHigh;
  _doAbsEtaHighCut = true;
  _theEtaLow = ptLow;
  _doAbsEtaLowCut = true;
  _doBothAbsEtaCut = true;
  return;
}
/**
 * @brief Determines whether an event satisfies the configured particle-trigger requirements.
 *
 * Evaluates the event's particle content against the configured trigger particle list and kinematic cuts and requires at least one matching particle for each configured trigger entry.
 *
 * @param e1 Pointer to the HepMC::GenEvent to evaluate.
 * @return true if every configured trigger particle type is present at least once in the event, `false` otherwise.
 */
bool HepMCParticleTrigger::isGoodEvent(HepMC::GenEvent* e1)
{
  // this is really just the call to actually evaluate and return the filter
  /*if (this->threshold == 0)
  {
    return true;
  }*/
  std::vector<int> n_trigger_particles = getParticles(e1);
  for (auto ntp : n_trigger_particles)
  {
    if (ntp <= 0)
    {
      return false;  // make sure all particles have at least 1
    }
  }
  return true;
}

/**
 * @brief Counts particles in a GenEvent that satisfy the configured stability and kinematic cuts.
 *
 * Scans the provided HepMC::GenEvent, applies the instance's stability (if enabled) and all enabled
 * eta, absolute-eta, pt, momentum (P), and Pz cuts, tallies surviving particles by absolute PDG ID,
 * and returns counts for each particle type requested by the trigger configuration.
 *
 * @param e1 Input HepMC::GenEvent to be scanned.
 * @return std::vector<int> A vector of counts where each element is the number of particles found
 *         matching the corresponding entry in the trigger's _theParticles list (same order). If a
 *         requested particle type is absent, its count is 0.
 */
std::vector<int> HepMCParticleTrigger::getParticles(HepMC::GenEvent* e1)
{
  std::vector<int> n_trigger{};
  std::map<int, int> particle_types;
  for (HepMC::GenEvent::particle_const_iterator iter = e1->particles_begin(); iter != e1->particles_end(); ++iter)
  {
    if (m_doStableParticleOnly && ((*iter)->end_vertex() || (*iter)->status() != 1))
    {
      continue;
    }
    auto p = (*iter)->momentum();
    float px = p.px();
    float py = p.py();
    float pz = p.pz();
    float p_M = std::sqrt(std::pow(px, 2) + std::pow(py, 2) + std::pow(pz, 2));
    float pt = std::sqrt(std::pow(px, 2) + std::pow(py, 2));
    int pid = std::abs((*iter)->pdg_id());
    double eta = p.eta();
    if ((_doEtaHighCut || _doBothEtaCut) && eta > _theEtaHigh)
    {
      continue;
    }
    if ((_doEtaLowCut || _doBothEtaCut) && eta < _theEtaLow)
    {
      continue;
    }
    if ((_doAbsEtaHighCut || _doBothAbsEtaCut) && std::abs(eta) > _theEtaHigh)
    {
      continue;
    }
    if ((_doAbsEtaLowCut || _doBothAbsEtaCut) && std::abs(eta) < _theEtaLow)
    {
      continue;
    }
    if ((_doPtHighCut || _doBothPtCut) && pt > _thePtHigh)
    {
      continue;
    }
    if ((_doPtLowCut || _doBothPtCut) && pt < _thePtLow)
    {
      continue;
    }
    if ((_doPHighCut || _doBothPCut) && p_M > _thePHigh)
    {
      continue;
    }
    if ((_doPLowCut || _doBothPCut) && p_M < _thePLow)
    {
      continue;
    }
    if ((_doPzHighCut || _doBothPzCut) && pz > _thePzHigh)
    {
      continue;
    }
    if ((_doPzLowCut || _doBothPzCut) && pz < _thePzLow)
    {
      continue;
    }
    if (particle_types.contains(pid))
    {
      particle_types[pid]++;
    }
    else
    {
      particle_types[pid] = 1;
    }
  }
  n_trigger.reserve(_theParticles.size());
  for (auto p : _theParticles)
  {
    n_trigger.push_back(particleAboveThreshold(particle_types, p));  // make sure we have at least one of each required particle
  }
  return n_trigger;
}
/**
 * @brief Retrieve the count of particles for a given trigger PDG identifier.
 *
 * @param n_particles Map from absolute PDG ID to the number of particles that passed cuts.
 * @param trigger_particle PDG ID to look up; its sign is ignored.
 * @return int The count for the absolute value of `trigger_particle` if present in `n_particles`, `0` otherwise.
 */
int HepMCParticleTrigger::particleAboveThreshold(const std::map<int, int>& n_particles, int trigger_particle)
{
  // search through for the number of identified trigger particles passing cuts
  auto it = n_particles.find(std::abs(trigger_particle));
  if (it != n_particles.end())
  {
    return it->second;
  }
  return 0;
}