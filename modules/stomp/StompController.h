#ifndef __MODULE_STOMPCONTROLLER_H
#define __MODULE_STOMPCONTROLLER_H

#include <sstream>
#include <string>
#include <map>

#include <netdb.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <openframe/openframe.h>

namespace stomp {
  using openframe::OpenFrame_Exception;
  using openframe::ListenController;
  using openframe::StringTool;
  using openframe::OFLock;
  using openframe::Connection;
  using openframe::Peer;
  using std::stringstream;
  using std::string;
  using std::map;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/
#define DEVICE_BINARY_SIZE  32
#define MAXPAYLOAD_SIZE     256

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class StompPeer;

  class StompController_Exception : public OpenFrame_Exception {
    public:
      StompController_Exception(const string message) throw() : OpenFrame_Exception(message) { };
  }; // class StompController_Exception

  class StompController : public ListenController {
    public:
      StompController(const int, const int, const time_t, const time_t, const time_t, const time_t);
      virtual ~StompController();

      /**********************
       ** Type Definitions **
       **********************/
      typedef map<int, StompPeer *> peers_t;
      typedef peers_t::iterator peers_itr;
      typedef peers_t::size_type peersSize_t;

      /***************
       ** Variables **
       ***************/
      static const time_t DEFAULT_STATS_INTERVAL;
      static const time_t CONNECT_RETRY_TIMEOUT;
      static const int HEADER_SIZE;

      // ### SocketController pure virtuals ###
      void onConnect(const Connection *);
      void onDisconnect(const Connection *);
      void onRead(const Peer *);
      const string::size_type onWrite(const Peer *, string &);

      const bool run();

    protected:
    private:
      peers_t _peers;
      OFLock _peers_l;
      time_t _sessionInitInterval;
      time_t _enquireLinkInterval;
      time_t _inactivityInterval;
      time_t _responseInterval;
  }; // StompController

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

  extern "C" {
  } // extern

} // namespace stomp
#endif
