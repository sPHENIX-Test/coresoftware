#ifndef FFAMODULES_SYNCRECO_H
#define FFAMODULES_SYNCRECO_H

#include <fun4all/SubsysReco.h>

#include <string>

/**
 * Construct a SyncReco module with an optional instance name.
 * @param name Instance name used to identify the module.
 */

/**
 * Initialize module-level resources and create required node structure if needed.
 * @param topNode Pointer to the top node of the Fun4All node tree.
 * @returns `0` on success, non-zero on error.
 */

/**
 * Perform run-specific initialization before processing events.
 * @param topNode Pointer to the top node of the Fun4All node tree.
 * @returns `0` on success, non-zero on error.
 */

/**
 * Process a single event.
 * @param topNode Pointer to the top node of the Fun4All node tree.
 * @returns `0` on success, non-zero on error.
 */

/**
 * Override the segment number used by the module.
 * @param i Segment number to force; use `-1` to unset the override.
 */

/**
 * Create the required node tree under the provided top node.
 * @param topNode Pointer to the top node of the Fun4All node tree.
 * @returns `0` on success, non-zero on error.
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