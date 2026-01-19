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
 * Configure which trigger indices are used to select events for skimming.
 * @param trigger_vector Vector of trigger indices to accept; the contents are copied into the internal trigger index list.
 */
void SetTrigger(std::vector<int> &trigger_vector) {m_trigger_index = trigger_vector;}

  /**
   * Enable a maximum-accept mode and configure the maximum number of events to accept.
   * @param max_events Maximum number of events to accept before further events are rejected.
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
 * Tracks the number of processed events as a zero-based counter starting at 0.
 */
int ievent{0};

  /**
 * @brief Count of events accepted by the skimmer.
 *
 * Tracks how many events have passed the skimmer's selection criteria. This
 * value is incremented when an event is accepted and is used together with
 * `max_accept` and `use_max_accept` to enforce a maximum number of accepted
 * events when that mode is enabled.
 */
int accepted_events{0};
  /**
 * Maximum number of events to accept when `use_max_accept` is enabled.
 *
 * Used as the threshold for `accepted_events`; only checked if `use_max_accept` is true.
 */
int max_accept{0};
  /**
 * Enable maximum-accept mode for event selection.
 *
 * When `true`, the skimmer will stop accepting events after `max_accept` events have been accepted.
 */
bool use_max_accept{false};

};

#endif // JETDSTSKIMMER_H