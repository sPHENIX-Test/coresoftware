// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef FUN4ALLRAW_FUN4ALLEVENTOUTSTREAM_H
#define FUN4ALLRAW_FUN4ALLEVENTOUTSTREAM_H

// base class for output streams writing Events in
// one or the other form

#include <fun4all/Fun4AllBase.h>

#include <Event/phenixTypes.h>

#include <boost/numeric/interval.hpp>

#include <map>
#include <string>

class Event;
class Packet;
class Fun4AllEventOutputManager;

class Fun4AllEventOutStream : public Fun4AllBase
{
 public:
  virtual ~Fun4AllEventOutStream();
  virtual int StreamStatus() { return 0; }
  virtual int WriteEvent(Event *evt);
  virtual int WriteEventOut(Event * /*evt*/) { return 0; }
  virtual int CloseOutStream() { return 0; }

  int AddPacket(const int ipkt);
  int DropPacket(const int ipkt);
  int AddPacketRange(const int minpacket, const int maxpacket);
  int DropPacketRange(const int minpacket, const int maxpacket);
  void SetManager(Fun4AllEventOutputManager *myman) { m_MyManager = myman; }

 protected:
  Fun4AllEventOutStream(const std::string &name = "OUTSTREAM");
  int resize_evtbuf(const unsigned int newsize);
  Fun4AllEventOutputManager *MyManager() { return m_MyManager; }

 private:
  PHDWORD *evtbuf;
  Fun4AllEventOutputManager *m_MyManager;  // pointer to my master
  unsigned int evtbuf_size;
  // flag to stear behavior, if 1 only add packets (drop all others), if 0 no filtering,
  // if -1 accept all, drop selected and afterwards add back selected ones
  int add_or_remove;
  Packet **plist;
  int max_npackets;
  int npackets;
  int default_addall;
  std::map<int, boost::numeric::interval<int> > addpktrange;
  std::map<int, boost::numeric::interval<int> > droppktrange;
};

#endif
