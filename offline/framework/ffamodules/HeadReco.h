// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef FFAMODULES_HEADRECO_H
#define FFAMODULES_HEADRECO_H

#include <fun4all/SubsysReco.h>

#include <string>  // for string

/**
 * HeadReco subsystem for event reconstruction within the FUN4ALL framework.
 *
 * Performs initialization and per-run setup, and processes each event to
 * execute the subsystem's reconstruction tasks.
 */
 
/**
 * Construct a HeadReco subsystem.
 *
 * @param name Optional name for the subsystem instance; defaults to "HeadReco".
 */
 
/**
 * Destroy the HeadReco subsystem.
 */
 
/**
 * Perform one-time initialization for the subsystem using the node tree.
 *
 * @param topNode Root node of the PHENIX/FUN4ALL node tree used to locate
 *                input/output nodes and services required by the subsystem.
 * @returns 0 on success, non-zero error code on failure.
 */
 
/**
 * Perform per-run initialization using the node tree.
 *
 * @param topNode Root node of the PHENIX/FUN4ALL node tree used to locate
 *                run-dependent nodes and configuration.
 * @returns 0 on success, non-zero error code on failure.
 */
 
/**
 * Process a single event using the node tree.
 *
 * @param topNode Root node of the PHENIX/FUN4ALL node tree containing event
 *                data and output containers manipulated by this subsystem.
 * @returns 0 on success, non-zero error code on failure.
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