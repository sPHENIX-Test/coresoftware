#ifndef FFAMODULES_SYNCRECO_H
#define FFAMODULES_SYNCRECO_H

#include <fun4all/SubsysReco.h>

#include <string>

/**
 * SyncReco subsystem reconstruction component responsible for synchronizing
 * event segmentation and providing lifecycle hooks for initialization and event processing.
 */

/**
 * Construct a SyncReco instance.
 * @param name Human-readable name for the instance (defaults to "SYNC").
 */

/**
 * Initialize module-level resources and create required nodes under @p topNode.
 * @param topNode Top-level node of the framework node tree.
 * @returns Status code: 0 on success, non-zero on error.
 */

/**
 * Prepare run-dependent resources before event processing begins.
 * @param topNode Top-level node of the framework node tree.
 * @returns Status code: 0 on success, non-zero on error.
 */

/**
 * Process a single event, performing synchronization-related actions using data under @p topNode.
 * @param topNode Top-level node of the framework node tree.
 * @returns Status code: 0 on success, non-zero on error.
 */

/**
 * Set a forced segment number to override the segment determined from input files.
 * @param i Segment number to force; use -1 to disable the override.
 */

/**
 * Create the required node tree structure under @p topNode for this module.
 * @param topNode Top-level node of the framework node tree.
 * @returns Status code: 0 on success, non-zero on error.
 */

/**
 * Optional forced segment override used to replace the segment number coming from input files;
 * a value of -1 indicates no override is applied.
 */
class SyncReco : public SubsysReco
{
 public:
  SyncReco(const std::string &name = "SYNC");
  ~SyncReco() override = default;

  int Init(PHCompositeNode *topNode) override;
  int InitRun(PHCompositeNode *topNode) override;
  int process_event(PHCompositeNode *topNode) override;

  void SegmentNumber(int i) { forced_segment = i; }

 private:
  static int CreateNodeTree(PHCompositeNode *topNode);
  // just if we need to override the segment for e.g. embedding
  // where we want to reuse hijing files which normally set
  // the segment number
  int forced_segment {-1};
};

#endif /* FFAMODULES_SYNCRECO_H */