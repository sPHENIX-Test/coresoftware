#include "CombineTowerInfo.h"

#include <calobase/TowerInfo.h>
#include <calobase/TowerInfoContainer.h>

#include <fun4all/Fun4AllReturnCodes.h>

#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/PHNodeIterator.h>
#include <phool/getClass.h>

#include <iostream>
#include <stdexcept>

/**
 * @brief Construct a CombineTowerInfo module with the given module name.
 *
 * @param name Identifier used to name this SubsysReco module instance.
 */
CombineTowerInfo::CombineTowerInfo(const std::string& name)
  : SubsysReco(name)
{
}

/**
 * @brief Initialize run-time nodes and validate input/output node names.
 *
 * Ensures that the configured input and output node names are set and then
 * creates or locates the required nodes under the provided top node.
 *
 * @param topNode Top-level node used to locate or create required DST nodes.
 * @return int Fun4AllReturnCodes::EVENT_OK on success.
 * @throws std::runtime_error If any of m_inputNodeA, m_inputNodeB, or m_outputNode is empty.
 */
int CombineTowerInfo::InitRun(PHCompositeNode* topNode)
{
  if (m_inputNodeA.empty() || m_inputNodeB.empty() || m_outputNode.empty())
  {
    throw std::runtime_error("CombineTowerInfo: input/output node names not set");
  }

  CreateNodes(topNode);
  return Fun4AllReturnCodes::EVENT_OK;
}

/**
 * @brief Locate input TowerInfoContainer nodes and initialize or create the output container.
 *
 * Retrieves the DST and detector nodes from the provided top-level node, obtains the two input
 * TowerInfoContainer instances (m_towersA and m_towersB) from their configured node names, and
 * sets up the output TowerInfoContainer (m_towersOut) under the detector node. If the output
 * container does not exist, it is created by cloning the first input container and attaching it
 * to the detector node.
 *
 * @param topNode Top-level PHCompositeNode used to search for DST, detector, input, and output nodes.
 *
 * @throws std::runtime_error If the DST node is not found, if either input container is missing,
 *                           or if the two input containers have different sizes.
 */
void CombineTowerInfo::CreateNodes(PHCompositeNode* topNode)
{
  PHNodeIterator iter(topNode);
  PHCompositeNode* dstNode =
      dynamic_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));

  if (!dstNode)
  {
    throw std::runtime_error("CombineTowerInfo: DST node not found");
  }

  PHCompositeNode *DetNode = dynamic_cast<PHCompositeNode *>(iter.findFirst("PHCompositeNode", m_detector));

  m_towersA = findNode::getClass<TowerInfoContainer>(topNode, m_inputNodeA);
  m_towersB = findNode::getClass<TowerInfoContainer>(topNode, m_inputNodeB);

  if (!m_towersB)
  {
    std::cout << "CombineTowerInfo: " <<m_inputNodeB << " not found" << std::endl;
    throw std::runtime_error("CombineTowerInfo: input TowerInfoContainer missing");
  }
  if (!m_towersA)
  {
    std::cout << "CombineTowerInfo: " <<m_inputNodeA << " not found" << std::endl;
    throw std::runtime_error("CombineTowerInfo: input TowerInfoContainer missing");
  }

  m_towersOut = findNode::getClass<TowerInfoContainer>(dstNode, m_outputNode);
  if (!m_towersOut)
  {
    m_towersOut =
        dynamic_cast<TowerInfoContainer*>(m_towersA->CloneMe());

    auto* node = new PHIODataNode<PHObject>(
        m_towersOut, m_outputNode, "PHObject");

    DetNode->addNode(node);
  }

  if (m_towersA->size() != m_towersB->size())
  {
    throw std::runtime_error("CombineTowerInfo: input containers have different sizes");
  }
}

/**
 * @brief Populate the output TowerInfoContainer by merging corresponding towers from the two inputs.
 *
 * For each tower index, copies the contents of the tower from input A into the output container
 * and sets the output tower's energy to the sum of the energies from input A and input B.
 *
 * @return int Fun4AllReturnCodes::EVENT_OK on success.
 */
int CombineTowerInfo::process_event(PHCompositeNode* /*topNode*/)
{
  const unsigned int ntowers = m_towersA->size();

  for (unsigned int ich = 0; ich < ntowers; ++ich)
  {
    TowerInfo* towerA = m_towersA->get_tower_at_channel(ich);
    TowerInfo* towerB = m_towersB->get_tower_at_channel(ich);
    TowerInfo* towerO = m_towersOut->get_tower_at_channel(ich);

    towerO->copy_tower(towerA);

    const float e_sum = towerA->get_energy() + towerB->get_energy();
    towerO->set_energy(e_sum);
  }

  return Fun4AllReturnCodes::EVENT_OK;
}
