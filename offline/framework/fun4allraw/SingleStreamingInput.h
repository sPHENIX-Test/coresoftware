#ifndef FUN4ALLRAW_SINGLESTREAMINGINPUT_H
#define FUN4ALLRAW_SINGLESTREAMINGINPUT_H

#include <fun4all/Fun4AllBase.h>
#include <fun4all/InputFileHandler.h>

#include <cstdint>  // for uint64_t
#include <map>
#include <set>
#include <string>

class Eventiterator;
class Fun4AllEvtInputPoolManager;
class Fun4AllStreamingInputManager;
class Fun4AllStreamingLumiCountingInputManager;
class PHCompositeNode;

class SingleStreamingInput : public Fun4AllBase, public InputFileHandler
{
 public:
  explicit SingleStreamingInput(const std::string &name);
  ~SingleStreamingInput() override;
  virtual Eventiterator *GetEventIterator() { return m_EventIterator; }
  virtual void FillPool(const uint64_t) { return; }
  virtual void FillPool(const unsigned int = 1) { return; }
  virtual void RunNumber(const int runno) { m_RunNumber = runno; }
  virtual int RunNumber() const { return m_RunNumber; }
  virtual int fileopen(const std::string &filename) override;
  virtual int fileclose() override;
  virtual int AllDone() const { return m_AllDone; }
  virtual void AllDone(const int i) { m_AllDone = i; }
  virtual void EventNumberOffset(const int i) { m_EventNumberOffset = i; }
  virtual void Print(const std::string &what = "ALL") const override;

  //! remove used packets matching a given BCO from internal container
  virtual void CleanupUsedPackets(const uint64_t /*BCO*/) {}

  //! remove used packets matching a given BCO from internal container
  /**
   * second parameter is to specify whether BCO has been
   * - succesfully processed or
   * - is dropped
   */
  virtual void CleanupUsedPackets(const uint64_t /*BCO*/ ,bool /*dropped*/) {}

  virtual bool CheckPoolDepth(const uint64_t bclk);
  virtual void ClearCurrentEvent();
  /**
 * Access the current Eventiterator.
 * @returns Pointer to the current Eventiterator, or `nullptr` if no iterator is set.
 */
virtual Eventiterator *GetEventiterator() const { return m_EventIterator; }
  /**
 * Get the current streaming input manager.
 *
 * @returns Pointer to the configured Fun4AllStreamingInputManager, or `nullptr` if no manager is set.
 */
virtual Fun4AllStreamingInputManager *StreamingInputManager() { return m_StreamingInputMgr; }
  /**
 * Assign the streaming input manager for this SingleStreamingInput instance.
 *
 * @param in Pointer to the Fun4AllStreamingInputManager to use; pass `nullptr` to clear the current manager.
 */
virtual void StreamingInputManager(Fun4AllStreamingInputManager *in) { m_StreamingInputMgr = in; }
  /**
 * Access the LumiCounting streaming input manager.
 *
 * @returns Pointer to the Fun4AllStreamingLumiCountingInputManager instance, or `nullptr` if no manager is set.
 */
virtual Fun4AllStreamingLumiCountingInputManager *StreamingLumiInputManager() { return m_StreamingLumiInputMgr; }
  /**
 * Set the streaming lumi counting input manager for this instance.
 * @param in Fun4AllStreamingLumiCountingInputManager to associate with this input.
 */
virtual void StreamingLumiInputManager(Fun4AllStreamingLumiCountingInputManager *in) { m_StreamingLumiInputMgr = in; }
  /**
 * Create DST node(s) under the provided composite node.
 *
 * Default implementation does nothing; subclasses should override to attach
 * DST-related nodes to the given top-level node when needed.
 * @param topNode Top-level PHCompositeNode to which DST nodes will be added.
 */
virtual void CreateDSTNode(PHCompositeNode *) { return; }
  /**
 * Configure the associated streaming input manager.
 *
 * Hook for subclasses to perform any manager-specific initialization or configuration.
 */
virtual void ConfigureStreamingInputManager() { return; }
  /**
 * Set the subsystem enumeration identifier for this input.
 * @param id Subsystem enumeration value identifying which subsystem this input corresponds to.
 */
virtual void SubsystemEnum(const int id) { m_SubsystemEnum = id; }
  virtual int SubsystemEnum() const { return m_SubsystemEnum; }
  void MaxBclkDiff(uint64_t ui) { m_MaxBclkSpread = ui; }
  uint64_t MaxBclkDiff() const { return m_MaxBclkSpread; }
  virtual const std::map<int, std::set<uint64_t>> &BclkStackMap() const { return m_BclkStackPacketMap; }
  virtual const std::set<uint64_t> &BclkStack() const { return m_BclkStack; }
  virtual const std::map<uint64_t, std::set<int>> &BeamClockFEE() const { return m_BeamClockFEE; }
  void setHitContainerName(const std::string &name) { m_rawHitContainerName = name; }
  const std::string &getHitContainerName() const { return m_rawHitContainerName; }
  const std::map<int, std::set<uint64_t>> &getFeeGTML1BCOMap() const { return m_FeeGTML1BCOMap; }

  void SetStandaloneMode(bool mode) { m_standalone_mode = mode; }
  bool IsStandaloneMode() const { return m_standalone_mode; }  
  //! event assembly QA histograms
  virtual void createQAHistos() {}

  //! event assembly QA for a given BCO
  /** TODO: check whether necessary */
  virtual void FillBcoQA(uint64_t /*gtm_bco*/) {};

  void clearPacketBClkStackMap(const uint64_t& bclk)

  {
    for(auto& [packetid, set] : m_BclkStackPacketMap)
    {
    
      for(auto it = set.begin(); it != set.end();)
    {
      if(*it <= bclk)
      {
        it = set.erase(it);
      }
      else
      {
        ++it;
      }
    }
    }
  }
  void clearFeeGTML1BCOMap(const uint64_t &bclk)
  {
    for (auto &[key, set] : m_FeeGTML1BCOMap)
    {
      for(auto it = set.begin(); it != set.end();)
      {
        if(*it <= bclk)
        {
          it = set.erase(it);
        }
        else
        {
          ++it;
        }
      }
      
    }
  }

 protected:
  std::map<int, std::set<uint64_t>> m_BclkStackPacketMap;
  std::map<int, std::set<uint64_t>> m_FeeGTML1BCOMap;
  std::string m_rawHitContainerName = "";
  bool m_standalone_mode = false;

 private:
  Eventiterator *m_EventIterator{nullptr};
  //  Fun4AllEvtInputPoolManager *m_InputMgr {nullptr};
  Fun4AllStreamingInputManager *m_StreamingInputMgr{nullptr};
  Fun4AllStreamingLumiCountingInputManager *m_StreamingLumiInputMgr{nullptr};

  uint64_t m_MaxBclkSpread{1000000};
  /**
 * Offset added to event numbers when assigning or reporting event IDs.
 *
 * Set to 1 by default; the effective event ID is the original event index plus this offset.
 */
unsigned int m_EventNumberOffset{1};  /**
 * Current run number for this input instance.
 *
 * A value of 0 indicates no run has been set. Packet event counters are zero-based,
 * but event numbering used here starts at 1 relative to those packet counters.
 */
  int m_RunNumber{0};
  int m_EventsThisFile{0};
  int m_AllDone{0};
  int m_SubsystemEnum{0};
  std::map<uint64_t, std::set<int>> m_BeamClockFEE;
  std::map<int, uint64_t> m_FEEBclkMap;
  std::set<uint64_t> m_BclkStack;
};

#endif