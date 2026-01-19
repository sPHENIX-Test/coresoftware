// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef TRIGGERDSTSKIMMER_H
#define TRIGGERDSTSKIMMER_H

#include <fun4all/SubsysReco.h>

#include <vector>
#include <string>

class PHCompositeNode;

class TriggerDSTSkimmer : public SubsysReco
{
 public:

  TriggerDSTSkimmer(const std::string &name = "TriggerDSTSkimmer");

  ~TriggerDSTSkimmer() override = default;

  int process_event(PHCompositeNode *topNode) override;

  /**
 * Set the list of trigger indices used to decide which events to accept.
 *
 * Copies the provided trigger indices into the internal trigger-index list,
 * replacing any previously set indices.
 *
 * @param trigger_vector Vector of trigger indices to use for event selection.
 */
void SetTrigger(std::vector<int> &trigger_vector) {m_trigger_index = trigger_vector;}

  /**
   * Enable the maximum-accept mode and configure the maximum number of accepted events.
   * @param max_events Maximum number of events to accept; when this limit is reached, the skimmer will stop accepting additional events.
   */
  void set_accept_max(int max_events) 
  {
    use_max_accept = true;
    max_accept = max_events;
    return;
  }

 private:

  std::vector<int> m_trigger_index{10}; 
  /**
 * Number of events processed by this skimmer instance.
 *
 * Incremented for each call to process_event to track the current event index.
 */
int ievent{0};

  /**
 * @brief Number of events accepted by the skimmer.
 *
 * Incremented each time an event passes selection; used to track progress and to enforce
 * the configured maximum number of accepted events when that limit is enabled.
 */
int accepted_events{0};
  /**
 * Maximum number of events to accept when maximum-accept mode is enabled.
 *
 * Initial value is 0. The limit is applied only if `use_max_accept` is set (via set_accept_max).
 */
int max_accept{0};
  /**
 * When true, the skimmer enforces the maximum accepted-events limit defined by `max_accept`.
 */
bool use_max_accept{false};

};

#endif // JETDSTSKIMMER_H