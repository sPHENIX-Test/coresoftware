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
 * @brief Construct a CombineTowerInfo subsystem instance.
 *
 * Initializes the SubsysReco base with the provided instance name.
 *
 * @param name Instance name for the subsystem (used as the SubsysReco name/identifier).
 */
CombineTowerInfo::CombineTowerInfo(const std::string& name)
  : SubsysReco(name)
{
}

/**
 * @brief Validate configured node names and initialize runtime nodes.
 *
 * Ensures input and output node names are set, creates or locates the required nodes
 * under the provided top-level node, and prepares internal containers for processing.
 *
 * @param topNode Top-level PHCompositeNode used to locate or attach DST and detector nodes.
 * @return int Fun4AllReturnCodes::EVENT_OK on success.
 * @throws std::runtime_error If any of the input or output node names are empty.
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
 * @brief Locate input TowerInfo containers, prepare the output container, and attach it to the node tree.
 *
 * Retrieves the input containers named by m_inputNodeA and m_inputNodeB from topNode, verifies both exist
 * and have the same size, and ensures an output TowerInfoContainer named by m_outputNode is present under
 * the detector node (cloning the A container if necessary). On creation, the output container is wrapped
 * in a PHIODataNode and added to the detector node.
 *
 * @param topNode Root node of the current PHENIX node tree used to find or create the required nodes.
 *
 * @throws std::runtime_error If the DST node is missing, if either input container is missing, or if the
 *                           input containers have different sizes.
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
 * @brief Merge tower data from the two input containers into the output container for the current event.
 *
 * For each tower channel, copies the tower contents from input A into the output and sets the output energy
 * to the sum of energies from input A and input B.
 *
 * @returns Fun4AllReturnCodes::EVENT_OK on success.
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
