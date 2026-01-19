#ifndef COMBINETOWERINFO_H
#define COMBINETOWERINFO_H

#include <fun4all/SubsysReco.h>

#include <string>

class PHCompositeNode;
class TowerInfoContainer;

class CombineTowerInfo : public SubsysReco
{
 public:
  explicit CombineTowerInfo(const std::string& name = "CombineTowerInfo");
  ~CombineTowerInfo() override = default;

  int InitRun(PHCompositeNode* topNode) override;
  int process_event(PHCompositeNode* topNode) override;

  /**
 * Set the name of the first input node used as a source of tower information.
 * @param name Node name for input A.
 */
void set_inputNodeA(const std::string& name) { m_inputNodeA = name; }
  /**
 * Set the name of the second input node.
 * @param name Name of the input node B from which tower information will be read.
 */
void set_inputNodeB(const std::string& name) { m_inputNodeB = name; }
  /**
 * Set the name of the output node where the combined TowerInfoContainer will be stored.
 * @param name Name of the output node for the combined TowerInfoContainer.
 */
void set_outputNode(const std::string& name) { m_outputNode = name; }
  /**
 * Set the detector name used to identify which detector's tower information to operate on.
 * @param name Detector name to assign to this instance.
 */
void set_detector(const std::string& name) { m_detector = name; }

 private:
  void CreateNodes(PHCompositeNode* topNode);

  std::string m_inputNodeA;
  std::string m_inputNodeB;
  std::string m_outputNode;
  std::string m_detector;

  TowerInfoContainer* m_towersA{nullptr};
  TowerInfoContainer* m_towersB{nullptr};
  /**
 * Container that will hold the combined tower information produced by this module.
 *
 * Initialized to nullptr and set to a valid TowerInfoContainer when the node tree is created.
 */
TowerInfoContainer* m_towersOut{nullptr};
};

#endif
