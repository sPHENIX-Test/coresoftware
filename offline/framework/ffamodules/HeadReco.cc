#include "HeadReco.h"

#include <ffaobjects/EventHeader.h>
#include <ffaobjects/EventHeaderv1.h>
#include <ffaobjects/RunHeader.h>
#include <ffaobjects/RunHeaderv1.h>

#include <phhepmc/PHHepMCGenEvent.h>
#include <phhepmc/PHHepMCGenEventMap.h>

#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/SubsysReco.h>  // for SubsysReco

#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>    // for PHIODataNode
#include <phool/PHNode.h>          // for PHNode
#include <phool/PHNodeIterator.h>  // for PHNodeIterator
#include <phool/PHObject.h>        // for PHObject
#include <phool/getClass.h>
#include <phool/recoConsts.h>

#include <Event/Event.h>

#include <HepMC/GenEvent.h>
#include <HepMC/HeavyIon.h>  // for HeavyIon

#include <iterator>  // for operator!=, reverse_iterator
#include <map>       // for _Rb_tree_iterator
#include <ranges>
#include <utility>  // for pair

HeadReco::HeadReco(const std::string &name)
  : SubsysReco(name)
{
}

// the nodes need to be created here since at least one input manager uses
// the event header. Creating them in InitRun() will be too late
int HeadReco::Init(PHCompositeNode *topNode)
{
  PHNodeIterator iter(topNode);
  PHCompositeNode *runNode = dynamic_cast<PHCompositeNode *>(iter.findFirst("PHCompositeNode", "RUN"));
  RunHeader *runheader = new RunHeaderv1();
  PHIODataNode<PHObject> *newNode = new PHIODataNode<PHObject>(runheader, "RunHeader", "PHObject");
  runNode->addNode(newNode);

  PHCompositeNode *dstNode = dynamic_cast<PHCompositeNode *>(iter.findFirst("PHCompositeNode", "DST"));
  EventHeader *eventheader = new EventHeaderv1();
  newNode = new PHIODataNode<PHObject>(eventheader, "EventHeader", "PHObject");
  dstNode->addNode(newNode);

  return Fun4AllReturnCodes::EVENT_OK;
}

int HeadReco::InitRun(PHCompositeNode *topNode)
{
  recoConsts *rc = recoConsts::instance();
  RunHeader *runheader = findNode::getClass<RunHeader>(topNode, "RunHeader");
  runheader->set_RunNumber(rc->get_IntFlag("RUNNUMBER"));
  return Fun4AllReturnCodes::EVENT_OK;
}

/**
 * @brief Populate the EventHeader from available event metadata nodes.
 *
 * Reads the PHHepMCGenEventMap (if present) and, for the first foreground
 * GenEvent encountered in reverse order, extracts HeavyIon information to set
 * ImpactParameter, EventPlaneAngle, FlowPsiN (n=1..6 when flow data exists),
 * eccentricity, Ncoll, and total Npart in the EventHeader. If the HepMC map is
 * absent, reads the PRDF Event node to set EvtType. Always sets RunNumber and
 * EvtSequence from the Fun4AllServer and, when verbosity is enabled, calls
 * identify() on the EventHeader.
 *
 * @param topNode Root of the node tree used to locate EventHeader, PHHepMCGenEventMap, and PRDF Event nodes.
 * @return int Fun4AllReturnCodes::EVENT_OK on success.
 */
int HeadReco::process_event(PHCompositeNode *topNode)
{
  Fun4AllServer *se = Fun4AllServer::instance();
  EventHeader *evtheader = findNode::getClass<EventHeader>(topNode, "EventHeader");
  PHHepMCGenEventMap *genevtmap = findNode::getClass<PHHepMCGenEventMap>(topNode, "PHHepMCGenEventMap");

  if (genevtmap)
  {
    for (auto &iter : std::ranges::reverse_view(*genevtmap))
    {
      PHHepMCGenEvent *genevt = iter.second;
      int embed_flag = genevt->get_embedding_id();
      if (embed_flag == 0)  // should be foreground event
      {
        HepMC::GenEvent *hepmcevt = genevt->getEvent();

        if (hepmcevt)
        {
          HepMC::HeavyIon *hi = hepmcevt->heavy_ion();
          if (hi)
          {
            evtheader->set_ImpactParameter(hi->impact_parameter());
            evtheader->set_EventPlaneAngle(hi->event_plane_angle());
	    if (! genevt->get_flow_psi_map().empty())
	    {
	      for (unsigned int n = 1; n <= 6; ++n)
	      {
		evtheader->set_FlowPsiN(n, genevt->get_flow_psi(n));
	      }
	    }
            evtheader->set_eccentricity(hi->eccentricity());
            evtheader->set_ncoll(hi->Ncoll());
            evtheader->set_npart(hi->Npart_targ() + hi->Npart_proj());
          }
        }
      }
    }
  }
  else
  {
    Event *evt = findNode::getClass<Event>(topNode, "PRDF");
    if (evt)
    {
      evtheader->set_EvtType(evt->getEvtType());
    }
  }
  evtheader->set_RunNumber(se->RunNumber());
  evtheader->set_EvtSequence(se->EventNumber());
  if (Verbosity() > 0)
  {
    evtheader->identify();
  }

  return Fun4AllReturnCodes::EVENT_OK;
}