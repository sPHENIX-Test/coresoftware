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
 * Set the input node name for tower container A.
 * @param name Name of the node to read TowerInfoContainer A from.
 */
void set_inputNodeA(const std::string& name) { m_inputNodeA = name; }
  /**
 * Set the name of the input node for tower container B.
 * @param name Name of the input node from which TowerInfoContainer B will be read.
 */
void set_inputNodeB(const std::string& name) { m_inputNodeB = name; }
  /**
 * Set the name of the output node where combined TowerInfo will be stored.
 * @param name Name of the output node.
 */
void set_outputNode(const std::string& name) { m_outputNode = name; }
  /**
 * Set the detector identifier used when combining tower information.
 * @param name Name of the detector.
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
 * @brief Pointer to the output TowerInfoContainer that stores combined tower information.
 *
 * Initialized to nullptr and set during initialization when the output node/container is created.
 */
TowerInfoContainer* m_towersOut{nullptr};
};

#endif
