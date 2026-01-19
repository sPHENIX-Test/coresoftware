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
 * @brief Construct a HepMCParticleTrigger with configurable trigger and cut defaults.
 *
 * Initializes the trigger threshold, optional event-count limit, and default kinematic
 * cut values and flags used to select particles (eta, absolute eta, pT, momentum, pz).
 *
 * @param trigger_thresh pT threshold used to enable and set the lower pT cut when nonzero.
 * @param n_incom Goal number of good events to collect when event limiting is enabled.
 * @param up_lim If true, stop processing after reaching the goal number of good events.
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
 * @brief Filter an input event using the configured HepMC particle trigger and update internal counters.
 *
 * Checks the PHHepMCGenEventMap on the provided node tree and verifies every contained HepMC::GenEvent
 * against the configured trigger criteria. Increments the total event counter and, for events that pass,
 * increments the accepted-event counter. Respects an optional configured limit on the number of accepted events.
 *
 * @param topNode Top-level node of the Fun4All/PHENIX node tree from which HepMC event data are retrieved.
 * @return int `Fun4AllReturnCodes::EVENT_OK` if the event passed the trigger and was processed; `Fun4AllReturnCodes::ABORTEVENT` if processing should be aborted
 * (no PHHepMCGenEventMap found, missing event data, the event failed the trigger, or the configured accepted-event limit has been reached).
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
 * @brief Add a particle PDG identifier to the trigger list.
 *
 * @param particlePid PDG code of the particle to include when counting trigger particles.
 */
void HepMCParticleTrigger::AddParticle(int particlePid)
{
  _theParticles.push_back(particlePid);
  return;
}
/**
 * @brief Adds multiple particle PDG IDs to the trigger list.
 *
 * Appends each PDG identifier from the provided vector to the module's internal list of trigger particles.
 *
 * @param particles Vector of particle PDG IDs to add.
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
 * @brief Configure the upper transverse momentum (pT) cut.
 *
 * Sets the pT upper bound used when filtering particles and enables the high-pT cut.
 *
 * @param pt Upper pT threshold (same units as particle momentum, typically GeV/c).
 *
 * If a pT lower cut is already enabled, this call also enables the combined pT range check.
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
 * @brief Set the lower transverse momentum (pT) threshold for the trigger and enable the pT low cut.
 *
 * Sets the lower pT bound used when selecting trigger particles and enables the pT-low filter. If a pT-high bound
 * has already been enabled, also enable the combined pT-high-and-low mode.
 *
 * @param pt Lower pT threshold (same units used throughout the module, e.g., GeV/c).
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
 * @brief Configure both upper and lower transverse momentum (pt) cuts.
 *
 * Enables the pt high and pt low selection and sets their thresholds.
 *
 * @param ptHigh Upper bound on transverse momentum (inclusive).
 * @param ptLow Lower bound on transverse momentum (inclusive).
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
 * @brief Set the upper bound for total momentum and enable the corresponding cut.
 *
 * Enables the momentum-high cut using the provided threshold. If a momentum-low cut
 * is already enabled, marks that both high and low momentum cuts should be applied.
 *
 * @param pt Upper momentum threshold (total momentum) to apply as the high cut.
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
 * @brief Set the lower bound for the particle total momentum cut.
 *
 * Sets the low momentum threshold and enables the P-low cut. If a P-high cut
 * is already enabled, also enable the combined P-high/P-low cut.
 *
 * @param pt Lower bound for the particle total momentum (P) to accept.
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
 * @brief Sets high and low thresholds for total momentum and enables momentum cuts.
 *
 * @param ptHigh Upper bound for the total momentum (p).
 * @param ptLow Lower bound for the total momentum (p).
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
 * @brief Set the upper bound for the longitudinal momentum (pz) cut.
 *
 * Sets the Pz high threshold to the given value and enables the Pz-high cut.
 * If a Pz-low cut is already enabled, also enables the combined Pz high+low cut.
 *
 * @param pt Upper bound for particle pz to pass the Pz cut (same units as input momenta).
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
 * @brief Set the lower bound for the longitudinal momentum (pz) cut and enable it.
 *
 * When a high-pz cut is already enabled, this also enables the combined pz-range cut.
 *
 * @param pt Lower pz threshold (inclusive).
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
 * @brief Set upper and lower bounds for the longitudinal momentum (pz) cut.
 *
 * Enables the pz high and low cuts and configures the module to apply both bounds.
 *
 * @param ptHigh Upper bound for |pz| (high threshold).
 * @param ptLow  Lower bound for |pz| (low threshold).
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
 * @brief Set the upper pseudorapidity (eta) cut and enable the corresponding cut flag.
 *
 * Enables the eta-high cut using the supplied upper bound and, if a eta-low cut is already active,
 * marks that both eta high and low cuts are in effect.
 *
 * @param pt Upper bound for pseudorapidity (eta).
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
 * @brief Set the lower bound for eta and enable the corresponding cut.
 *
 * Enables the eta low cut and, if an eta high cut is already enabled, also enables the combined eta-range cut.
 *
 * @param pt Lower eta bound to apply (inclusive).
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
 * @brief Configure both upper and lower pseudorapidity (eta) bounds and enable their checks.
 *
 * Sets the eta upper bound to @p ptHigh and the eta lower bound to @p ptLow, and enables
 * the corresponding high, low, and combined eta cut flags so particles will be tested
 * against both limits.
 *
 * @param ptHigh Upper pseudorapidity bound (eta).
 * @param ptLow Lower pseudorapidity bound (eta).
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
 * @brief Set the upper bound for absolute pseudorapidity and enable the corresponding cut.
 *
 * Configures the absolute-eta high threshold to `pt` and activates the absolute-eta high cut.
 * If an absolute-eta low cut is already active, also enables the combined absolute-eta range cut.
 *
 * @param pt Upper bound on |eta|.
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
 * @brief Configure the lower bound for absolute pseudorapidity and enable the corresponding cut.
 *
 * Sets the absolute-eta lower threshold and enables the absolute-eta-low cut; if an absolute-eta-high
 * cut is already enabled, enables the combined absolute-eta high/low cut.
 *
 * @param pt Lower bound on absolute pseudorapidity (|eta|).
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
 * @brief Set absolute pseudorapidity bounds and enable the corresponding cuts.
 *
 * Sets the upper and lower limits for |eta| and activates both the absolute-eta
 * high and low cuts.
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
 * @brief Determines whether the given HepMC event satisfies the configured trigger.
 *
 * Evaluates the event using the active kinematic and stability cuts and verifies that each
 * configured trigger particle type is present at least once after those cuts.
 *
 * @param e1 HepMC event to evaluate.
 * @return `true` if every configured trigger particle has a count greater than zero, `false` otherwise.
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
 * @brief Count trigger particle occurrences in a HepMC event after applying configured cuts.
 *
 * Scans the provided GenEvent and counts particles that satisfy the module's configured
 * stability and kinematic selection criteria (eta/|eta|, pt, momentum magnitude, pz).
 * The returned vector contains the observed counts for each PDG id in the module's
 * configured trigger list, preserving that order.
 *
 * @param e1 Pointer to the HepMC::GenEvent to inspect.
 * @return std::vector<int> Counts of matching particles for each configured trigger PDG id;
 *         `0` for a requested PDG id if no matching particles are found.
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
 * @brief Retrieve the observed count for a requested trigger particle PDG id.
 *
 * Looks up the absolute value of the provided `trigger_particle` in `n_particles`
 * and returns the associated count of particles that passed the configured cuts.
 *
 * @param n_particles Map from absolute PDG id to observed count after cuts.
 * @param trigger_particle PDG id to query; the sign is ignored (absolute value used).
 * @return int Count for the requested particle PDG id if present, `0` otherwise.
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