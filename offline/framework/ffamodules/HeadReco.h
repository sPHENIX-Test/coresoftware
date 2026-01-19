// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef FFAMODULES_HEADRECO_H
#define FFAMODULES_HEADRECO_H

#include <fun4all/SubsysReco.h>

#include <string>  // for string

/**
 * Reconstruction module for the Head detector that integrates with the SubsysReco framework.
 */

/**
 * Create a HeadReco module with an optional instance name.
 * @param name Instance name for this module (defaults to "HeadReco").
 */

/**
 * Initialize module resources before event processing.
 * @param topNode Root node of the framework's node tree.
 * @returns 0 on success, non-zero on error.
 */

/**
 * Perform per-run initialization at the start of a run.
 * @param topNode Root node of the framework's node tree.
 * @returns 0 on success, non-zero on error.
 */

/**
 * Process a single event.
 * @param topNode Root node of the framework's node tree.
 * @returns 0 on success, non-zero on error.
 */
class HeadReco : public SubsysReco
{
 public:
  HeadReco(const std::string &name = "HeadReco");
  ~HeadReco() override = default;
  int Init(PHCompositeNode *topNode) override;
  int InitRun(PHCompositeNode *topNode) override;
  int process_event(PHCompositeNode *topNode) override;

 protected:
};

#endif
