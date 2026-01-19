#include "CopyIODataNodes.h"

#include <globalvertex/GlobalVertexMapv1.h>
#include <globalvertex/GlobalVertexMap.h>

#include <calotrigger/MinimumBiasInfo.h>

#include <centrality/CentralityInfo.h>

#include <calobase/TowerInfoContainer.h>
#include <calobase/TowerInfo.h>

#include <mbd/MbdOut.h>

#include <ffaobjects/EventHeader.h>
#include <ffaobjects/RunHeader.h>
#include <ffaobjects/SyncObject.h>

#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/Fun4AllServer.h>

#include <phool/PHCompositeNode.h>
#include <phool/getClass.h>

//____________________________________________________________________________..
CopyIODataNodes::CopyIODataNodes(const std::string &name)
  : SubsysReco(name)
{
}

/**
 * @brief Initialize per-run IO data nodes in the destination topology.
 *
 * For each enabled copy flag, ensure the corresponding node(s) exist in the Fun4AllServer top node
 * by creating or initializing destination nodes based on the input topology provided by topNode.
 *
 * @param topNode Source PHCompositeNode containing run-level nodes to copy or clone from.
 * @return int Fun4All return code; `Fun4AllReturnCodes::EVENT_OK` on success.
 */
int CopyIODataNodes::InitRun(PHCompositeNode *topNode)
{
  Fun4AllServer *se = Fun4AllServer::instance();
  if (m_CopyRunHeaderFlag)
  {
    CopyRunHeader(topNode, se->topNode());
  }
  if (m_CopyEventHeaderFlag)
  {
    CreateEventHeader(topNode, se->topNode());
  }
  if (m_CopyCentralityInfoFlag)
  {
    CreateCentralityInfo(topNode, se->topNode());
  }
  if (m_CopyGlobalVertexMapFlag)
  {
    CreateGlobalVertexMap(topNode, se->topNode());
  }
  if (m_CopyMinimumBiasInfoFlag)
  {
    CreateMinimumBiasInfo(topNode, se->topNode());
  }
  if (m_CopyMbdOutFlag)
  {
    CreateMbdOut(topNode, se->topNode());
  }
  if (m_CopySyncObjectFlag)
  {
    CreateSyncObject(topNode, se->topNode());
  }
  if (m_CopyTowerInfoFlag)
  {
    CreateTowerInfo(topNode, se->topNode());
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

/**
 * @brief Perform per-event copying of selected data nodes from the input node tree to the server's top node.
 *
 * For the current event, conditionally copies configured node types (event header, centrality info,
 * global vertex map, minimum-bias info, MBD output, sync object, and tower info) from the provided
 * input top node into the Fun4All server's output topology.
 *
 * @param topNode Source top-level node for this event from which data nodes will be copied.
 * @return Fun4AllReturnCodes::EVENT_OK on successful processing of the event.
 */
int CopyIODataNodes::process_event(PHCompositeNode *topNode)
{
  Fun4AllServer *se = Fun4AllServer::instance();
  if (m_CopyEventHeaderFlag)
  {
    CopyEventHeader(topNode, se->topNode());
  }
  if (m_CopyCentralityInfoFlag)
  {
    CopyCentralityInfo(topNode, se->topNode());
  }
  if (m_CopyGlobalVertexMapFlag)
  {
    CopyGlobalVertexMap(topNode, se->topNode());
  }
  if (m_CopyMinimumBiasInfoFlag)
  {
    CopyMinimumBiasInfo(topNode, se->topNode());
  }
  if (m_CopyMbdOutFlag)
  {
    CopyMbdOut(topNode, se->topNode());
  }
  if (m_CopySyncObjectFlag)
  {
    CopySyncObject(topNode, se->topNode());
  }
  if (m_CopyTowerInfoFlag)
  {
    CopyTowerInfo(topNode, se->topNode());
  }
  return Fun4AllReturnCodes::EVENT_OK;
}

void CopyIODataNodes::CopyRunHeader(PHCompositeNode *from_topNode, PHCompositeNode *to_topNode)
{
  RunHeader *from_runheader = findNode::getClass<RunHeader>(from_topNode, "RunHeader");
  if (!from_runheader)
  {
    std::cout << "Could not locate RunHeader on " << from_topNode->getName() << std::endl;
    m_CopyRunHeaderFlag = false;
    return;
  }
  RunHeader *to_runheader = findNode::getClass<RunHeader>(to_topNode, "RunHeader");
  if (!to_runheader)
  {
    PHNodeIterator iter(to_topNode);
    PHCompositeNode *runNode = dynamic_cast<PHCompositeNode *>(iter.findFirst("PHCompositeNode", "RUN"));
    if (!runNode)
    {
      runNode = new PHCompositeNode("RUN");
      to_topNode->addNode(runNode);
    }
    to_runheader = dynamic_cast<RunHeader *>(from_runheader->CloneMe());
    PHIODataNode<PHObject> *newNode = new PHIODataNode<PHObject>(to_runheader, "RunHeader", "PHObject");
    runNode->addNode(newNode);
    if (Verbosity() > 0)
    {
      std::cout << "From RunHeader identify()" << std::endl;
      from_runheader->identify();
      std::cout << "To RunHeader identify()" << std::endl;
      to_runheader->identify();
    }
  }
  return;
}

void CopyIODataNodes::CreateCentralityInfo(PHCompositeNode *from_topNode, PHCompositeNode *to_topNode)
{
  CentralityInfo *from_centralityinfo = findNode::getClass<CentralityInfo>(from_topNode, "CentralityInfo");
  if (!from_centralityinfo)
  {
    std::cout << "Could not locate CentralityInfo on " << from_topNode->getName() << std::endl;
    m_CopyCentralityInfoFlag = false;
    return;
  }
  CentralityInfo *to_centralityinfo = findNode::getClass<CentralityInfo>(to_topNode, "CentralityInfo");
  if (!to_centralityinfo)
  {
    PHNodeIterator iter(to_topNode);
    PHCompositeNode *dstNode = dynamic_cast<PHCompositeNode *>(iter.findFirst("PHCompositeNode", "DST"));
    if (!dstNode)
    {
      dstNode = new PHCompositeNode("DST");
      to_topNode->addNode(dstNode);
    }
    to_centralityinfo = dynamic_cast<CentralityInfo *>(from_centralityinfo->CloneMe());
    PHIODataNode<PHObject> *newNode = new PHIODataNode<PHObject>(to_centralityinfo, "CentralityInfo", "PHObject");
    dstNode->addNode(newNode);
  }
  return;
}

void CopyIODataNodes::CopyCentralityInfo(PHCompositeNode *from_topNode, PHCompositeNode *to_topNode)
{
  CentralityInfo *from_centralityinfo = findNode::getClass<CentralityInfo>(from_topNode, "CentralityInfo");
  CentralityInfo *to_centralityinfo = findNode::getClass<CentralityInfo>(to_topNode, "CentralityInfo");
  from_centralityinfo->CopyTo(to_centralityinfo);
  if (Verbosity() > 0)
  {
    std::cout << "From CentralityInfo identify()" << std::endl;
    from_centralityinfo->identify();
    std::cout << "To CentralityInfo identify()" << std::endl;
    to_centralityinfo->identify();
  }

  return;
}

void CopyIODataNodes::CreateEventHeader(PHCompositeNode *from_topNode, PHCompositeNode *to_topNode)
{
  EventHeader *from_eventheader = findNode::getClass<EventHeader>(from_topNode, "EventHeader");
  if (!from_eventheader)
  {
    std::cout << "Could not locate EventHeader on " << from_topNode->getName() << std::endl;
    m_CopyEventHeaderFlag = false;
    return;
  }
  EventHeader *to_eventheader = findNode::getClass<EventHeader>(to_topNode, "EventHeader");
  if (!to_eventheader)
  {
    PHNodeIterator iter(to_topNode);
    PHCompositeNode *dstNode = dynamic_cast<PHCompositeNode *>(iter.findFirst("PHCompositeNode", "DST"));
    if (!dstNode)
    {
      dstNode = new PHCompositeNode("DST");
      to_topNode->addNode(dstNode);
    }
    to_eventheader = dynamic_cast<EventHeader *>(from_eventheader->CloneMe());
    PHIODataNode<PHObject> *newNode = new PHIODataNode<PHObject>(to_eventheader, "EventHeader", "PHObject");
    dstNode->addNode(newNode);
  }
  return;
}

void CopyIODataNodes::CopyEventHeader(PHCompositeNode *from_topNode, PHCompositeNode *to_topNode)
{
  EventHeader *from_eventheader = findNode::getClass<EventHeader>(from_topNode, "EventHeader");
  EventHeader *to_eventheader = findNode::getClass<EventHeader>(to_topNode, "EventHeader");
  from_eventheader->CopyTo(to_eventheader);
  if (Verbosity() > 0)
  {
    std::cout << "From EventHeader identify()" << std::endl;
    from_eventheader->identify();
    std::cout << "To EventHeader identify()" << std::endl;
    to_eventheader->identify();
  }

  return;
}

void CopyIODataNodes::CreateGlobalVertexMap(PHCompositeNode *from_topNode, PHCompositeNode *to_topNode)
{
  GlobalVertexMap *from_globalvertexmap = findNode::getClass<GlobalVertexMap>(from_topNode, "GlobalVertexMap");
  if (!from_globalvertexmap)
  {
    std::cout << "Could not locate GlobalVertexMap on " << from_topNode->getName() << std::endl;
    m_CopyGlobalVertexMapFlag = false;
    return;
  }
  GlobalVertexMap *to_globalvertexmap = findNode::getClass<GlobalVertexMap>(to_topNode, "GlobalVertexMap");
  if (!to_globalvertexmap)
  {
    PHNodeIterator iter(to_topNode);
    PHCompositeNode *dstNode = dynamic_cast<PHCompositeNode *>(iter.findFirst("PHCompositeNode", "DST"));
    if (!dstNode)
    {
      dstNode = new PHCompositeNode("DST");
      to_topNode->addNode(dstNode);
    }
    to_globalvertexmap = new GlobalVertexMapv1();
    PHIODataNode<PHObject> *newNode = new PHIODataNode<PHObject>(to_globalvertexmap, "GlobalVertexMap", "PHObject");
    dstNode->addNode(newNode);
  }
  return;
}

void CopyIODataNodes::CopyGlobalVertexMap(PHCompositeNode *from_topNode, PHCompositeNode *to_topNode)
{
  GlobalVertexMap *from_globalvertexmap = findNode::getClass<GlobalVertexMap>(from_topNode, "GlobalVertexMap");
  GlobalVertexMap *to_globalvertexmap = findNode::getClass<GlobalVertexMap>(to_topNode, "GlobalVertexMap");
  from_globalvertexmap->CopyTo(to_globalvertexmap);
  if (Verbosity() > 0)
  {
    std::cout << "From GlobalVertexMap identify()" << std::endl;
    from_globalvertexmap->identify();
    std::cout << "To GlobalVertexMap identify()" << std::endl;
    to_globalvertexmap->identify();
  }

  return;
}

void CopyIODataNodes::CreateMinimumBiasInfo(PHCompositeNode *from_topNode, PHCompositeNode *to_topNode)
{
  MinimumBiasInfo *from_minimumbiasinfo = findNode::getClass<MinimumBiasInfo>(from_topNode, "MinimumBiasInfo");
  if (!from_minimumbiasinfo)
  {
    std::cout << "Could not locate MinimumBiasInfo on " << from_topNode->getName() << std::endl;
    m_CopyMinimumBiasInfoFlag = false;
    return;
  }
  MinimumBiasInfo *to_minimumbiasinfo = findNode::getClass<MinimumBiasInfo>(to_topNode, "MinimumBiasInfo");
  if (!to_minimumbiasinfo)
  {
    PHNodeIterator iter(to_topNode);
    PHCompositeNode *dstNode = dynamic_cast<PHCompositeNode *>(iter.findFirst("PHCompositeNode", "DST"));
    if (!dstNode)
    {
      dstNode = new PHCompositeNode("DST");
      to_topNode->addNode(dstNode);
    }
    to_minimumbiasinfo = dynamic_cast<MinimumBiasInfo *>(from_minimumbiasinfo->CloneMe());
    PHIODataNode<PHObject> *newNode = new PHIODataNode<PHObject>(to_minimumbiasinfo, "MinimumBiasInfo", "PHObject");
    dstNode->addNode(newNode);
  }
  return;
}

/**
 * @brief Copies MinimumBiasInfo contents from the source to the destination node tree.
 *
 * If both source and destination MinimumBiasInfo objects exist, the source's data is copied
 * into the destination. When verbosity is greater than zero, identify() is printed for both objects.
 *
 * @param from_topNode Top-level node containing the source MinimumBiasInfo.
 * @param to_topNode Top-level node containing the destination MinimumBiasInfo.
 */
void CopyIODataNodes::CopyMinimumBiasInfo(PHCompositeNode *from_topNode, PHCompositeNode *to_topNode)
{
  MinimumBiasInfo *from_minimumbiasinfo = findNode::getClass<MinimumBiasInfo>(from_topNode, "MinimumBiasInfo");
  MinimumBiasInfo *to_minimumbiasinfo = findNode::getClass<MinimumBiasInfo>(to_topNode, "MinimumBiasInfo");
  from_minimumbiasinfo->CopyTo(to_minimumbiasinfo);
  if (Verbosity() > 0)
  {
    std::cout << "From MinimumBiasInfo identify()" << std::endl;
    from_minimumbiasinfo->identify();
    std::cout << "To MinimumBiasInfo identify()" << std::endl;
    to_minimumbiasinfo->identify();
  }

  return;
}

/**
 * @brief Copy per-channel tower data from the source TowerInfoContainer to the destination container.
 *
 * Copies each tower present in the source container named by from_towerInfo_name into the
 * corresponding channel of the destination container named by to_towerInfo_name. The number
 * of channels copied equals the size() of the source container.
 *
 * @param from_topNode Top-level node containing the source TowerInfoContainer.
 * @param to_topNode Top-level node containing the destination TowerInfoContainer.
 */
void CopyIODataNodes::CopyTowerInfo(PHCompositeNode *from_topNode, PHCompositeNode *to_topNode)
{
  TowerInfoContainer *from_towerInfo = findNode::getClass<TowerInfoContainer>(from_topNode, from_towerInfo_name);
  TowerInfoContainer   *to_towerInfo = findNode::getClass<TowerInfoContainer>(  to_topNode, to_towerInfo_name);
  unsigned int ntowers = from_towerInfo->size();
  for (unsigned int ch = 0; ch < ntowers; ++ch)
  {
    TowerInfo *from_tow = from_towerInfo->get_tower_at_channel(ch);
    to_towerInfo->get_tower_at_channel(ch)->copy_tower(from_tow);
  }
  
  if (Verbosity() > 0)
  {
    std::cout << "From TowerInfoContainer identify()" << std::endl;
    from_towerInfo->identify();
    std::cout << "To TowerInfoCOntainer identify()" << std::endl;
    to_towerInfo->identify();
  }

  return;
}


/**
 * @brief Ensure a MbdOut node exists under the destination topology by cloning it from the source.
 *
 * If a MbdOut object exists in the source topology, this routine ensures the destination contains
 * a DST/MBD composite path and attaches a cloned MbdOut wrapped in a PHIODataNode under MBD.
 * If no MbdOut is found in the source, the corresponding copy flag (m_CopyMbdOutFlag) is disabled
 * and a diagnostic message is emitted.
 *
 * @param from_topNode Source root node to copy MbdOut from.
 * @param to_topNode Destination root node to receive the cloned MbdOut.
 */
void CopyIODataNodes::CreateMbdOut(PHCompositeNode *from_topNode, PHCompositeNode *to_topNode)
{

  MbdOut *from_mbdout = findNode::getClass<MbdOut>(from_topNode, "MbdOut");
  if (!from_mbdout)
  {
    std::cout << "Could not locate MbdOut no " << from_topNode->getName() << std::endl;
    m_CopyMbdOutFlag = false;
    return;
  }
  MbdOut *to_mbdout = findNode::getClass<MbdOut>(to_topNode, "MbdOut");
  if (!to_mbdout)
  {
    PHNodeIterator iter(to_topNode);
    PHCompositeNode *dstNode = dynamic_cast<PHCompositeNode *>(iter.findFirst("PHCompositeNode", "DST"));
    if (!dstNode)
    {
      dstNode = new PHCompositeNode("DST");
      to_topNode->addNode(dstNode);
    }

    PHNodeIterator dstiter(dstNode);
    PHCompositeNode *mbdNode = dynamic_cast<PHCompositeNode *>(dstiter.findFirst("PHCompositeNode", "MBD"));
    if (!mbdNode)
    {
      mbdNode = new PHCompositeNode("MBD");
      dstNode->addNode(mbdNode);
    }

    to_mbdout = dynamic_cast<MbdOut *>(from_mbdout->CloneMe());
    PHIODataNode<PHObject> *newNode = new PHIODataNode<PHObject>(to_mbdout, "MbdOut", "PHObject");
    mbdNode->addNode(newNode);
  }
  return;

}

/**
 * @brief Ensure a TowerInfoContainer exists under the destination node tree by cloning it from the source if necessary.
 *
 * If a TowerInfoContainer named by the configured source identifier cannot be found on from_topNode,
 * the function disables further tower-info copying (m_CopyTowerInfoFlag = false) and returns.
 * If the destination does not already contain a TowerInfoContainer with the configured destination name,
 * the function ensures a DST composite node exists under to_topNode, clones the source TowerInfoContainer,
 * wraps it in a PHIODataNode, and attaches it under DST.
 *
 * @param from_topNode Root node of the source topology to copy the TowerInfoContainer from.
 * @param to_topNode Root node of the destination topology where the TowerInfoContainer will be created if missing.
 */
void CopyIODataNodes::CreateTowerInfo(PHCompositeNode *from_topNode, PHCompositeNode *to_topNode)
{
  std::cout << "copying tower info" << std::endl;
  TowerInfoContainer *from_towerInfo = findNode::getClass<TowerInfoContainer>(from_topNode, from_towerInfo_name);
  if (!from_towerInfo)
  {
    std::cout << "Could not locate TowerInfoContainer on " << from_topNode->getName() << std::endl;
    m_CopyTowerInfoFlag = false;
    return;
  }
  TowerInfoContainer *to_towerInfo = findNode::getClass<TowerInfoContainer>(to_topNode, to_towerInfo_name);
  if (!to_towerInfo)
  {
    PHNodeIterator iter(to_topNode);
    PHCompositeNode *dstNode = dynamic_cast<PHCompositeNode *>(iter.findFirst("PHCompositeNode", "DST"));
    if (!dstNode)
    {
      dstNode = new PHCompositeNode("DST");
      to_topNode->addNode(dstNode);
    }
    to_towerInfo = dynamic_cast<TowerInfoContainer*>(from_towerInfo->CloneMe());
    PHIODataNode<PHObject> *newNode = new PHIODataNode<PHObject>(to_towerInfo, to_towerInfo_name, "PHObject");
    dstNode->addNode(newNode);
  }
  return;
}




/**
 * @brief Copy MBD output data from one node tree into another.
 *
 * Locates the MbdOut object in the source and destination PHCompositeNode trees
 * and copies the contents from the source into the destination. If verbosity
 * is enabled, prints identify() information for both source and destination
 * MbdOut objects.
 *
 * @param from_topNode Top node of the source tree containing the original MbdOut.
 * @param to_topNode Top node of the destination tree to receive the copied MbdOut.
 */
void CopyIODataNodes::CopyMbdOut(PHCompositeNode *from_topNode, PHCompositeNode *to_topNode)
{
  MbdOut *from_mbdout = findNode::getClass<MbdOut>(from_topNode, "MbdOut");
  MbdOut *to_mbdout = findNode::getClass<MbdOut>(to_topNode, "MbdOut");
  from_mbdout->CopyTo(to_mbdout);
  if (Verbosity() > 0)
  {
    std::cout << "From MbdOut identify()" << std::endl;
    from_mbdout->identify();
    std::cout << "To MbdOut identify()" << std::endl;
    to_mbdout->identify();
  }
  
  return;
}

void CopyIODataNodes::CreateSyncObject(PHCompositeNode *from_topNode, PHCompositeNode *to_topNode)
{
  SyncObject *from_syncobject = findNode::getClass<SyncObject>(from_topNode, "Sync");
  if (!from_syncobject)
  {
    std::cout << "Could not locate SyncObject on " << from_topNode->getName() << std::endl;
    m_CopySyncObjectFlag = false;
    return;
  }
  SyncObject *to_syncobject = findNode::getClass<SyncObject>(to_topNode, "Sync");
  if (!to_syncobject)
  {
    PHNodeIterator iter(to_topNode);
    PHCompositeNode *dstNode = dynamic_cast<PHCompositeNode *>(iter.findFirst("PHCompositeNode", "DST"));
    if (!dstNode)
    {
      dstNode = new PHCompositeNode("DST");
      to_topNode->addNode(dstNode);
    }
    to_syncobject = dynamic_cast<SyncObject *>(from_syncobject->CloneMe());
    PHIODataNode<PHObject> *newNode = new PHIODataNode<PHObject>(to_syncobject, "SyncObject", "PHObject");
    dstNode->addNode(newNode);
  }
  return;
}

void CopyIODataNodes::CopySyncObject(PHCompositeNode *from_topNode, PHCompositeNode *to_topNode)
{
  SyncObject *from_syncobject = findNode::getClass<SyncObject>(from_topNode, "Sync");
  SyncObject *to_syncobject = findNode::getClass<SyncObject>(to_topNode, "Sync");
  to_syncobject = from_syncobject;
  if (Verbosity() > 0)
  {
    std::cout << "From Syncobject identify()" << std::endl;
    from_syncobject->identify();
    std::cout << "To Syncobject identify()" << std::endl;
    to_syncobject->identify();
  }
  return;
}