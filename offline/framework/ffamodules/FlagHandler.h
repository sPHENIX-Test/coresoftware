// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef FFAMODULES_FLAGHANDLER_H
#define FFAMODULES_FLAGHANDLER_H

#include <fun4all/SubsysReco.h>

#include <string>

/**
 * Construct a FlagHandler module with an optional name.
 * @param name Human-readable name for the module instance.
 */
/**
 * Destroy the FlagHandler module.
 */
/**
 * Ensure the Flag node exists; if it does, read flags and copy them into reconstruction constants.
 * @param topNode Root node of the framework node tree to create or read the Flag node from.
 * @returns 0 on success, non-zero on error.
 */
/**
 * Perform end-of-processing cleanup for the module.
 * @param topNode Root node of the framework node tree.
 * @returns 0 on success, non-zero on error.
 */
/**
 * Print information about the module.
 * @param what Selector for which information to print (defaults to "ALL").
 */
/**
 * Update or create run-level nodes related to flags during processing.
 * @param topNode Root node of the framework node tree to update.
 * @returns 0 on success, non-zero on error.
 */
class FlagHandler : public SubsysReco
{
 public:
  FlagHandler(const std::string &name = "FlagHandler");

  ~FlagHandler() override = default;

  /** Create the Flag Node if it does not exist,
      if it exists, read back flags and copy them into recoConsts
   */
  int InitRun(PHCompositeNode *topNode) override;

  /// Called at the end of all processing.
  int End(PHCompositeNode *topNode) override;

  void Print(const std::string &what = "ALL") const override;

  int UpdateRunNode(PHCompositeNode *topNode) override;

 private:
};

#endif  // FFAMODULES_FLAGHANDLER_H