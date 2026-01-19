#ifndef FUN4ALLRAW_SINGLETRIGGEREDINPUT_H
#define FUN4ALLRAW_SINGLETRIGGEREDINPUT_H

#include <fun4all/Fun4AllBase.h>
#include <fun4all/InputFileHandler.h>

#include <Event/EventTypes.h>
#include <Event/Eventiterator.h>
#include <Event/fileEventiterator.h>
#include <Event/packet.h>

#include <array>
#include <cstdint>  // for uint64_t
#include <deque>
#include <fstream>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <vector>

class Event;
class Eventiterator;
class OfflinePacket;
class PHCompositeNode;

class SingleTriggeredInput : public Fun4AllBase, public InputFileHandler
{
 public:
  static constexpr size_t pooldepth{10};  // number of events which are read in in one go
  explicit SingleTriggeredInput(const std::string &name);
  ~SingleTriggeredInput() override;
  virtual Eventiterator *GetEventIterator() { return m_EventIterator; }
  virtual void FillPool();
  /**
 * Set the current run number used for subsequent event processing.
 * @param runno The run number to assign to this input handler.
 */
virtual void RunNumber(const int runno) { m_RunNumber = runno; }
  virtual int RunNumber() const { return m_RunNumber; }
  virtual int fileopen(const std::string &filename) override;
  /**
   * Close the currently opened input file and release associated resources.
   * @returns `0` on success, non-zero on failure.
   */
  /**
   * Get the completion status flag for this input.
   * @returns Non-zero if processing is complete, `0` if not complete.
   */
  /**
   * Set the completion status flag for this input.
   * @param i Non-zero to mark processing as complete, `0` to mark as not complete.
   */
  virtual int fileclose() override;
  virtual int AllDone() const { return m_AllDone; }
  virtual void AllDone(const int i) { m_AllDone = i; }
  virtual int FilesDone() const { return m_FilesDone; }
  /**
 * Set the number of files that have been processed by this input handler.
 * @param i The count of files processed to store internally.
 */
virtual void FilesDone(const int i) { m_FilesDone = i; }
  /**
 * Set the event alignment problem flag or counter.
 * @param i New value for the event alignment problem flag or count.
 */
virtual void EventAlignmentProblem(const int i) { m_EventAlignmentProblem = i; }
  virtual int EventAlignmentProblem() const { return m_EventAlignmentProblem; }
  /**
 * Set the current event number used by the input handler.
 * @param i Event number to store as the current event index.
 */
virtual void EventNumber(const int i) { m_EventNumber = i; }
  virtual int EventNumber() const { return m_EventNumber; }
  virtual void CreateDSTNodes(Event *evt);
  // these ones are used directly by the derived classes, maybe later
  // move to cleaner accessors
  virtual int FillEventVector();
  virtual void FillPacketClock(Event *evt, Packet *pkt, size_t event_index);
  virtual int ReadEvent();
  virtual SingleTriggeredInput *Gl1Input() { return m_Gl1Input; }
  virtual void Gl1Input(SingleTriggeredInput *input) { m_Gl1Input = input; }
  virtual uint64_t GetClock(Event *evt, int pid);
  virtual std::array<uint64_t, pooldepth>::const_iterator clkdiffbegin() { return m_bclkdiffarray.begin(); }
  virtual std::array<uint64_t, pooldepth>::const_iterator clkdiffend() { return m_bclkdiffarray.end(); }
  virtual std::array<uint64_t, pooldepth>::const_iterator beginclock() { return m_bclkarray.begin(); }
  virtual void KeepPackets() { m_KeepPacketsFlag = true; }
  virtual bool KeepMyPackets() const { return m_KeepPacketsFlag; }
  void topNode(PHCompositeNode *topNode) { m_topNode = topNode; }
  PHCompositeNode *topNode() { return m_topNode; }
  virtual void FakeProblemEvent(const int ievent) { m_ProblemEvent = ievent; }
  virtual int FemEventNrClockCheck(OfflinePacket *calopkt);
  void dumpdeque();
  int checkfirstsebevent();
  virtual bool CheckFemDiffIdx(int pid, size_t index, const std::deque<Event*>& events, uint64_t gl1diffidx);
  virtual bool CheckPoolAlignment(int pid, const std::array<uint64_t, pooldepth>& sebdiff, const std::array<uint64_t, pooldepth>& gl1diff, std::vector<int>& bad_indices, int& shift, bool& CurrentPoolLastDiffBad, bool PrevPoolLastDiffBad);
  virtual bool FemClockAlignment(int pid, const std::deque<Event*>& events, const std::array<uint64_t, pooldepth>& gl1diff);

 protected:
  PHCompositeNode *m_topNode{nullptr};
  // lined up like this:
  // Event * | previous event beam clock | clock diff to previous event
  // keeping previous beam clock just eases the looping, we want to be able to have
  // the accompanying diff to the previous beam clock with this event, so any mismatch
  // gives us the event index in the deque which is off
  std::deque<Event *> m_EventDeque;
  std::map<int, std::deque<Event*>> m_PacketEventDeque;
  std::map<int, Event*> m_PacketEventBackup;
  std::map<int, int> m_PacketShiftOffset;
  std::array<uint64_t, pooldepth + 1> m_bclkarray{};  // keep the last bco from previous loop
  std::array<uint64_t, pooldepth> m_bclkdiffarray{};
  std::map<int, std::array<uint64_t, pooldepth + 1>> m_bclkarray_map;
  std::map<int, std::array<uint64_t, pooldepth>>     m_bclkdiffarray_map;
  std::set<int> m_PacketSet;
  static uint64_t ComputeClockDiff(uint64_t curr, uint64_t prev) { return (curr - prev) & 0xFFFFFFFF; }


 private:
  Eventiterator *m_EventIterator{nullptr};
  SingleTriggeredInput *m_Gl1Input{nullptr};
  /**
 * Overall completion status flag for the input handler.
 *
 * 0 indicates processing is ongoing; any non-zero value indicates processing is complete.
 */
int m_AllDone{0};
  /**
 * @brief Running total of processed events.
 *
 * Incremented for each processed event and used as a global event counter/identifier for this input handler.
 */
uint64_t m_Event{0};
  /**
 * Current event number within the input stream used for tracking processed events.
 *
 * This integer is incremented as events are read and represents the sequential
 * index (event count) for the input source.
 */
int m_EventNumber{0};
  /**
 * Count of event alignment problems encountered.
 *
 * Tracks how many alignment or clock-consistency issues have been detected for processed events; a value of 0 indicates no problems.
 */
int m_EventAlignmentProblem{0};
  /**
 * Number of input files that have been processed.
 *
 * Maintains a running count of completed files for this input handler; initialized to 0.
 */
int m_FilesDone{0};
  int m_LastEvent{std::numeric_limits<int>::max()};
  int m_ProblemEvent{-1};
  int m_RepresPacket{-1};
  int m_RunNumber{0};
  int m_max_alignment_retries{5};
  bool firstcall{true};
  bool firstclockcheck{true};
  bool m_KeepPacketsFlag{false};
  /**
 * If true, copy beam-clock information from representative packets into other packets during run processing.
 */
/**
 * Set of packet IDs for which the copied clock has been verified as correct.
 */
/**
 * Map of packet ID -> set of event indices that should be skipped ("ditched") for that packet.
 */
/**
 * Set of FEM event numbers that have been observed or tracked.
 */
/**
 * Set of packet IDs that should have their clocks overridden with the representative packet's clock.
 */
/**
 * Map of packet ID -> number of consecutive alignment failures encountered for that packet.
 */
/**
 * Map of packet ID -> flag indicating whether an alignment problem has been recorded for that packet.
 */
/**
 * Map of packet ID -> flag indicating whether the last difference in the previous pool was considered bad for that packet.
 */
/**
 * Map of packet ID -> previously known valid beam clock origin (BCO) value for that packet.
 */
/**
 * Monotonic counter of processed events across the lifetime of this input instance.
 */
bool m_packetclk_copy_runs{false};
  std::set<int> m_CorrectCopiedClockPackets;
  std::map<int, std::set<int>> m_DitchPackets;
  std::set<int> m_FEMEventNrSet;
  std::set<int> m_OverrideWithRepClock;
  std::map<int, int> m_PacketAlignmentFailCount;
  std::map<int, bool> m_PacketAlignmentProblem;
  std::map<int, bool> m_PrevPoolLastDiffBad;
  std::map<int, uint64_t> m_PreviousValidBCOMap;
  long long eventcounter{0};
};

#endif