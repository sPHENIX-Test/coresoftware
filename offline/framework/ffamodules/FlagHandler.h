// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef FFAMODULES_FLAGHANDLER_H
#define FFAMODULES_FLAGHANDLER_H

#include <fun4all/SubsysReco.h>

#include <string>

/**
 * Ensure the Flag node exists in the node tree and synchronize any existing flags
 * into the reconstruction constants.
 * @param topNode Top-level node of the run's node tree.
 * @returns Status code: `0` on success, non-zero on failure.
 */
/**
 * Perform end-of-processing tasks for the FlagHandler.
 * @param topNode Top-level node of the run's node tree.
 * @returns Status code: `0` on success, non-zero on failure.
 */
/**
 * Print FlagHandler information filtered by the specified scope.
 * @param what Scope of information to print (default: "ALL").
 */
/**
 * Update the run node with the current flag state.
 * @param topNode Top-level node of the run's node tree.
 * @returns Status code: `0` on success, non-zero on failure.
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