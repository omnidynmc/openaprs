#include <fstream>
#include <string>
#include <queue>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <list>
#include <map>
#include <new>
#include <iostream>
#include <fstream>

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <netdb.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>

#include <openframe/openframe.h>

#include "StompController.h"
#include "StompPeer.h"

#include "stomp.h"

namespace stomp {
  using namespace openframe;
  using namespace std;
  using openframe::StringTool;

/**************************************************************************
 ** StompController Class                                                 **
 **************************************************************************/
  const time_t StompController::CONNECT_RETRY_TIMEOUT 	= 60;
  const time_t StompController::DEFAULT_STATS_INTERVAL 	= 3600;

  StompController::StompController(const int port, const int max, const time_t sessionInitInterval, const time_t enquireLinkInterval, const time_t inactivityInterval, const time_t responseInterval) :
    ListenController(port, max), _sessionInitInterval(sessionInitInterval), _enquireLinkInterval(enquireLinkInterval), _inactivityInterval(inactivityInterval), _responseInterval(responseInterval) {

    return;
  } // StompController::StompController

  StompController::~StompController() {

    return;
  } // StompController::~StompController

  const bool StompController::run() {
    peers_itr ptr;

    _peers_l.Lock();
    for(ptr = _peers.begin(); ptr != _peers.end(); ptr++) {
      StompPeer *peer = ptr->second;
      peer->run();
    } // for
    _peers_l.Unlock();

    return true;
  } // StompController::run

  void StompController::onConnect(const Connection *con) {
    StompPeer *peer;

    cout << "### STOMP Connected to " << derive_ip(con->sock) << ":" << derive_port(con->sock) << endl;

    try {
      peer = new StompPeer(con->sock, _sessionInitInterval, _enquireLinkInterval, _inactivityInterval, _responseInterval, 10);
    } // try
    catch(bad_alloc xa) {
      assert(false);
    } // catch

    _peers_l.Lock();
    _peers.insert( make_pair(con->sock, peer) );
    _peers_l.Unlock();

    return;
  } // StompController::onConnect

  void StompController::onDisconnect(const Connection *con) {
    peers_itr ptr;

    cout << "### STOMP Disconnected from " << derive_ip(con->sock) << ":" << derive_port(con->sock) << endl;

    _peers_l.Lock();
    ptr = _peers.find(con->sock);
    if (ptr != _peers.end())
      _peers.erase(ptr);
    _peers_l.Unlock();

    return;
  } // StompController::onDisconnect

  void StompController::onRead(const Peer *lis) {
    peers_itr ptr;

    stringstream out;
    out << "[ STOMP Packet from " << derive_peer_ip(lis->sock) << ":" << derive_peer_port(lis->sock) << " ]";
    cout << "<" << StringTool::ppad(out.str(), "-", 79) << endl;

    _peers_l.Lock();
    ptr = _peers.find(lis->sock);
    if (ptr != _peers.end()) {
      cout << StringTool::hexdump(lis->in, "<   ") << "<" << endl;
      ptr->second->receive(lis->in);
    } // if
    _peers_l.Unlock();

    return;
  } // StompController::onRead

  const string::size_type StompController::onWrite(const Peer *lis, string &ret) {
    peers_itr ptr;

    _peers_l.Lock();
    ptr = _peers.find(lis->sock);
    if (ptr != _peers.end()) {
      ptr->second->transmit(ret);
      if (ret.length()) {
        stringstream out;
        out << "[ STOMP Packet to " << derive_peer_ip(lis->sock) << ":" << derive_peer_port(lis->sock) << " ]";
        cout << StringTool::ipad(out.str(), "-", 79) << ">" << endl;
//        cout << pad << endl;
        cout << StringTool::hexdump(ret, ">   ") << ">" << endl;
      } // if
    } // if
    _peers_l.Unlock();

    return ret.size();
  } // StompController::onWrite

} // namespace apns

