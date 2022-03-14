#include <cassert>
#include <exception>
#include <iostream>
#include <new>
#include <string>

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <openframe/openframe.h>

class SocketServer : public openframe::ListenController {
  public:
    SocketServer(const int port, const int maxclients) : openframe::ListenController(port, maxclients) { }
    virtual ~SocketServer() { }

    virtual void onConnect(const openframe::Connection *con) {
      std::cout << "Connected to " << con->host_str << std::endl;
    } // onConnect
    virtual void onConnectTimeout(const openframe::Peer *peer) {
      std::cout << "Connect read timeout to " << peer->host_str << std::endl;
    } // onConnectTimeout
    virtual void onDisconnect(const openframe::Connection *con) {
      std::cout << "Disconnected from " << con->host_str << std::endl;
    } // onDisconnect
    virtual void onRead(const openframe::Peer *peer) {
      openframe::scoped_lock slock(&in_l);
      std::string buf;
      buf.assign(peer->in, peer->in_len);
      in.push(buf);
    } // onRead

    virtual void onPeerStats(const openframe::Peer *peer) {
      std::cout << peer->peer_stats_str() << std::endl;
    } // onPeerStats
    virtual const std::string::size_type onWrite(const openframe::Peer *peer, std::string &ret) { return 0; }
//    virtual bool onTick() { return false; }
//    virtual bool onPeerWake(const openframe::Peer *peer) { return false; }

    std::queue<std::string> in;
    openframe::OFLock in_l;
  protected:
  private:
}; // class Feed

int main(int argc, char **argv) {
  SocketServer *sockserv = new SocketServer(10152, 1024);

  sockserv->start();

  const time_t stats_intval = 5;
  const time_t sleep_intval = 2000;

  time_t last_stats = time(NULL);
  size_t num_in = 0;
  size_t num_bytes = 0;

  while(true) {
    bool did_work = false;
    while( !sockserv->in.empty() ) {
      std::string buf = sockserv->in.front();
//      std::cout << "IN(" << buf << ")" << std::endl;
      sockserv->in.pop();
      num_in++;
      num_bytes += buf.length();
      did_work |= true;
    } // while

    if (last_stats < time(NULL) - stats_intval) {
      time_t diff = time(NULL) - last_stats;
      double pps = double(num_in) / double(diff);
      double bps = double(num_bytes) / double(diff);
      std::cout << "pps=" << std::fixed << std::setprecision(2) << pps
                << ",bps=" << std::fixed << std::setprecision(2) << bps << std::endl;
      num_in = 0;
      num_bytes = 0;
      last_stats = time(NULL);
    } // if
    if (!did_work) usleep(sleep_intval);
  } // while

  sockserv->stop();

  delete sockserv;
  exit(0);
} // main
