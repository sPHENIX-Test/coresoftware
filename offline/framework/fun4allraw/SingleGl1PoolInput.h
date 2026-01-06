#ifndef FUN4ALLRAW_SINGLEGL1POOLINPUT_H
#define FUN4ALLRAW_SINGLEGL1POOLINPUT_H

#include "SingleStreamingInput.h"

#include <cstdint>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

class Gl1Packet;
class PHCompositeNode;

/**
 * Construct a SingleGl1PoolInput with the given name.
 * @param name Identifier used to name this input instance.
 */
/**
 * Destroy the SingleGl1PoolInput and release associated resources.
 */
/**
 * Populate the internal packet pool using up to the given number of items.
 * @param count Maximum number of packets or items to attempt to add to the pool.
 */
/**
 * Remove packets from internal structures that have been consumed up to the specified BCLK.
 * @param bclk Boundary clock value; packets with timestamps <= bclk are eligible for cleanup.
 */
/**
 * Verify whether the internal pool depth is sufficient relative to the specified BCLK.
 * @param bclk Clock value used as the reference for pool depth checks.
 * @returns `true` if the pool contains enough data for the provided BCLK, `false` otherwise.
 */
/**
 * Reset state associated with the current event so the input can start a new event.
 */
/**
 * Attempt to acquire additional events into the internal pool or event buffers.
 * @returns `true` if one or more new events were obtained, `false` if no further events are available.
 */
/**
 * Print status or diagnostic information.
 * @param what Selector string indicating which information to print (default: "ALL").
 */
/**
 * Create and attach necessary DST nodes to the provided top-level node.
 * @param topNode Parent node in the PHENIX node tree where new DST nodes will be created.
 */
/**
 * Set the acceptable BCO range used by this input.
 * @param i BCO range value to apply.
 */
/**
 * Set the size of the negative BCO window (how far before a reference BCO to include).
 * @param value Number of BCO units to include before the reference BCO.
 */
/**
 * Set the size of the positive BCO window (how far after a reference BCO to include).
 * @param value Number of BCO units to include after the reference BCO.
 */
/**
 * Set the maximum number of events this input should produce.
 * @param value Maximum total events to produce; use a large value to indicate no practical limit.
 */
class SingleGl1PoolInput : public SingleStreamingInput
{
 public:
  explicit SingleGl1PoolInput(const std::string &name);
  ~SingleGl1PoolInput() override;
  void FillPool(const unsigned int) override;
  void CleanupUsedPackets(const uint64_t bclk) override;
  bool CheckPoolDepth(const uint64_t bclk) override;
  void ClearCurrentEvent() override;
  bool GetSomeMoreEvents();
  void Print(const std::string &what = "ALL") const override;
  void CreateDSTNode(PHCompositeNode *topNode) override;
  void SetBcoRange(const unsigned int i) { m_BcoRange = i; }
  //  void ConfigureStreamingInputManager() override;
  void SetNegativeWindow(const unsigned int value) { m_negative_bco_window = value; }
  void SetPositiveWindow(const unsigned int value) { m_positive_bco_window = value; }
  void SetTotalEvent(const int value) { m_total_event = value; }
 private:
  unsigned int m_NumSpecialEvents{0};
  unsigned int m_BcoRange{0};
  unsigned int m_negative_bco_window = 20;
  unsigned int m_positive_bco_window = 325;
  int m_total_event = std::numeric_limits<int>::max();
  bool m_alldone_flag = {false};
  bool m_lastevent_flag = {false};
  //! map bco to packet
  std::map<unsigned int, uint64_t> m_packet_bco;

  std::map<uint64_t, std::pair<uint64_t, uint64_t>> m_BCOWindows;
  std::map<uint64_t, int> m_BCOBunchNumber;
  std::map<uint64_t, std::vector<Gl1Packet *>> m_Gl1RawHitMap;
  std::set<uint64_t> m_FEEBclkMap;
  std::set<uint64_t> m_BclkStack;
};

#endif