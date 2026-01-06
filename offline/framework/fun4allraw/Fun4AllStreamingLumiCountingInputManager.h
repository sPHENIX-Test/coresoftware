// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef RAWBCOLUMI_FUN4ALLSTREAMINGLUMICOUNTINGINPUTMANAGER_H
#define RAWBCOLUMI_FUN4ALLSTREAMINGLUMICOUNTINGINPUTMANAGER_H

#include <fun4allraw/InputManagerType.h>
// #include <fun4allraw/Fun4AllStreamingInputManager.h>
#include <fun4all/Fun4AllInputManager.h>

#include <TTree.h>
#include <map>
#include <set>
#include <string>
class SingleStreamingInput;
class Gl1Packet;
class PHCompositeNode;
class SyncObject;
class TH1;
class TTree;
/**
   * Construct a streaming luminosity-counting input manager.
   * @param name Instance name used for identification.
   * @param dstnodename Name of the DST node to which outputs attach.
   * @param topnodename Name of the top-level node in the node tree.
   */
  /**
   * Clean up resources used by the input manager.
   */
  /**
   * Close any open input resources and release associated state.
   * @returns `0` on success, non-zero on failure.
   */
  /**
   * Run processing loop for up to `nevents` events.
   * @param nevents Maximum number of events to process; `0` means process until exhaustion or external stop.
   * @returns Number of events actually processed, or a negative error code.
   */
  /**
   * Print internal status or diagnostics.
   * @param what Selector controlling what to print (implementation-defined); defaults to "ALL".
   */
  /**
   * Reset the current event state so the manager is ready to read or process the next event.
   * @returns `0` on success, non-zero on failure.
   */
  /**
   * Increase the number of events the manager should process or buffer.
   * @param i Number of events to add to the internal backlog.
   * @returns New total of backlog events on success, negative on failure.
   */
  /**
   * Provide the caller with the manager's synchronization object.
   * @param mastersync Output pointer that will receive the manager's SyncObject.
   * @returns `0` on success, non-zero on failure.
   */
  /**
   * Synchronize this input manager to the provided master synchronization object.
   * @param mastersync The master SyncObject to synchronize against.
   * @returns `0` on success, non-zero on failure.
   */
  /**
   * Retrieve a string value or property associated with this manager.
   * @param what Key identifying the requested property or information.
   * @returns The requested string value, or an empty string if not available.
   */
  /**
   * Register a streaming input source with this manager.
   * @param evtin Pointer to the streaming input to register.
   * @param enu_subsystem Enum value indicating the subsystem the input belongs to.
   */
  /**
   * Process accumulated GL1 raw hits and update internal luminosity/counting structures.
   * @returns `0` on success, non-zero on failure.
   */
  /**
   * Add a raw GL1 hit associated with a BCLK value.
   * @param bclk Bunch clock (BCO) or timestamp identifying the hit's bunch crossing.
   * @param hit Pointer to the Gl1Packet representing the raw hit; ownership is not transferred by this call.
   */
  /**
   * Register a valid GL1 window around a trimmed BCO.
   * @param bco_trim Trimmed BCO value used as the window center.
   * @param negative_window Number of bunch crossings included before `bco_trim`.
   * @param positive_window Number of bunch crossings included after `bco_trim`.
   */
  /**
   * Record the bunch number associated with a trimmed BCO.
   * @param bco_trim Trimmed BCO value.
   * @param bunch_number Bunch number to associate with `bco_trim`.
   */
  /**
   * Set the size of the negative-side BCO window used when matching hits.
   * @param i Number of bunch crossings before the center to include.
   */
  /**
   * Set the size of the positive-side BCO window used when matching hits.
   * @param i Number of bunch crossings after the center to include.
   */
  /**
   * Enable or disable streaming mode for this manager.
   * @param b If `true`, enable streaming behaviour; if `false`, disable it.
   */
  class Fun4AllStreamingLumiCountingInputManager : public Fun4AllInputManager
{
 public:
  Fun4AllStreamingLumiCountingInputManager(const std::string &name = "DUMMY", const std::string &dstnodename = "DST", const std::string &topnodename = "TOP");
  ~Fun4AllStreamingLumiCountingInputManager() override;
  int fileopen(const std::string & /*filenam*/) override { return 0; }
  // cppcheck-suppress virtualCallInConstructor
  int fileclose() override;
  int run(const int nevents = 0) override;

  void Print(const std::string &what = "ALL") const override;
  int ResetEvent() override;
  int PushBackEvents(const int i) override;
  int GetSyncObject(SyncObject **mastersync) override;
  int SyncIt(const SyncObject *mastersync) override;
  int HasSyncObject() const override { return 1; }
  std::string GetString(const std::string &what) const override;
  void registerStreamingInput(SingleStreamingInput *evtin, InputManagerType::enu_subsystem);
  int FillGl1();
  void AddGl1RawHit(uint64_t bclk, Gl1Packet *hit);
  void AddGl1Window(uint64_t bco_trim, int negative_window, int positive_window);
  void AddGl1BunchNumber(uint64_t bco_trim, int bunch_number);
  void SetNegativeWindow(const unsigned int i);
  void SetPositiveWindow(const unsigned int i);
  void Streaming(bool b = true) { m_StreamingFlag = b; }
  void SetOutputFileName(const std::string &fileName);
  void SetEndofEvent(bool flag = false, bool flag2 = false)
  {
    m_alldone_flag = flag;
    m_lastevent_flag = flag2;
  }
  void SetEventNumber(int num) { m_event_number = num; }

 private:
  struct Gl1RawHitInfo
  {
    std::vector<Gl1Packet *> Gl1RawHitVector;
    unsigned int EventFoundCounter{0};
  };

  void createLuminosityHistos();

  SyncObject *m_SyncObject{nullptr};
  PHCompositeNode *m_topNode{nullptr};

  int m_RunNumber{0};
  unsigned int m_negative_bco_window{0};
  unsigned int m_positive_bco_window{0};
  uint64_t m_rawgl1scaler{0};
  //  std::string m_output_file="output.root";
  bool m_alldone_flag = {false};
  bool m_lastevent_flag = {false};
  int m_event_number{0};
  int m_diffBCO{0};
  bool m_gl1_registered_flag{false};
  bool m_StreamingFlag{false};
  bool flat_overflow{false};
  uint64_t bco_temp = 0;

  std::vector<SingleStreamingInput *> m_Gl1InputVector;
  std::map<uint64_t, Gl1RawHitInfo> m_Gl1RawHitMap;
  std::map<uint64_t, std::pair<uint64_t, uint64_t>> m_BCOWindows;
  std::map<uint64_t, int> m_BCOBunchNumber;
  std::map<int, long> m_bunchnumber_MBDNS_raw;
  std::map<int, long> m_bunchnumber_MBDNS_live;
  std::map<int, long> m_bunchnumber_MBDNS_scaled;
  std::map<int, long> m_bunchnumber_ZDCCoin_raw;
  // std::map<int, long> m_bunchnumber_rawgl1scaler;

  // QA histos
  TH1 *h_lumibco{nullptr};
  TH1 *h_bunchnumber{nullptr};
  TH1 *h_bunchnumber_occur{nullptr};
  TH1 *h_diffbco{nullptr};
  TH1 *h_gl1p_MBDSN_bunchid_raw{nullptr};
  TH1 *h_gl1p_MBDSN_bunchid_live{nullptr};
  TH1 *h_gl1p_MBDSN_bunchid_scaled{nullptr};
  TH1 *h_gl1p_rawgl1scaler{nullptr};
  TH1 *h_gl1p_ZDCCoin_bunchid_raw{nullptr};
  uint64_t m_bco_trim{};
  uint64_t m_lower_bound{};
  uint64_t m_upper_bound{};
  int m_bunch_number{};
  TTree *ttree = nullptr;
  TFile *tfile = nullptr;
  std::string m_outputFileName = "/sphenix/user/xuzhiwan/luminosity/streaming-macro/macro/output.root";  // Default value
};

#endif /* RAWBCOLUMI_FUN4ALLSTREAMINGLUMICOUNTINGINPUTMANAGER_H */