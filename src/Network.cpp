/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, Internet APRS MySQL Injector                               **
 ** Copyright (C) 1999 Gregory A. Carter                                 **
 **                    Daniel Robert Karrels                             **
 **                    Dynamic Networking Solutions                      **
 **                                                                      **
 ** This program is free software; you can redistribute it and/or modify **
 ** it under the terms of the GNU General Public License as published by **
 ** the Free Software Foundation; either version 1, or (at your option)  **
 ** any later version.                                                   **
 **                                                                      **
 ** This program is distributed in the hope that it will be useful,      **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of       **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        **
 ** GNU General Public License for more details.                         **
 **                                                                      **
 ** You should have received a copy of the GNU General Public License    **
 ** along with this program; if not, write to the Free Software          **
 ** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            **
 **************************************************************************
 $Id: Network.cpp,v 1.2 2006/03/21 20:55:12 omni Exp $
 **************************************************************************/

#include <fstream>
#include <string>
#include <queue>
#include <cstdlib>
#include <cctype>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <list>
#include <map>
#include <new>
#include <iostream>

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <netdb.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>

#include "zlib.h"

#include "OpenAPRS_Abstract.h"
#include "Server.h"
#include "Network.h"

#include "config.h"
#include "openaprs.h"

namespace openaprs {

using std::string;
using std::map;
using std::list;
using std::pair;
using std::cout;

/**************************************************************************
 ** Connection Class                                                     **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

Connection::Connection() {
  // set defaults
  myCompress = false;
  _closed = false;
  _retain = false;

  setWriteNextTS(0);
  setSendBufferSize(NETWORK_MTU);
  setCompress(false);
  myPacketCount = 0;
  myFd = 0;
} // Connection::Connection

Connection::~Connection() {
  if (myFd > 0)
    ::close(myFd);

  if (getCompress() == true) {
    inflateEnd(&myDecompStream);
    deflateEnd(&myCompStream);
  } // if
} // Connection~Connection

/***************
 ** Variables **
 ***************/

const Connection::flagType Connection::FLAG_ZLIB	=	0x0001;
const Connection::flagType Connection::FLAG_BINARY	=	0x0002;

/**********************
 ** Compress Members **
 **********************/

const int Connection::blocking(const bool blocking) {
  int flags = fcntl(myFd, F_GETFL, 0);

  if (flags == -1)
    flags = 0;

  if (blocking && flags & O_NONBLOCK) {
    if (fcntl(myFd, F_SETFL, flags | ~O_NONBLOCK) == -1)
      return NETWORK_ERROR;
  } // if
  else if (flags & ~O_NONBLOCK) {
    if (fcntl(myFd, F_SETFL, flags | O_NONBLOCK) == -1)
      return NETWORK_ERROR;
  } // else

  return NETWORK_TRUE;
} // Connection::blocking

const int Connection::compressBuffer(Bytef *compressFrom, const int compressFromSize) {
  myCompStream.next_in = compressFrom;
  myCompStream.avail_in = compressFromSize;

  // cout << "compressFrom(" << (char *) compressFrom << ")" << endl;

  while ((myCompStream.avail_in > 0) || (myCompStream.avail_out < NETWORK_MTU)) {
    myCompStream.next_out = myCBuffer;
    myCompStream.avail_out = NETWORK_MTU;
    int zlibCode = deflate(&myCompStream, Z_SYNC_FLUSH);

    if (zlibCode == Z_STREAM_ERROR || zlibCode == Z_DATA_ERROR) {
      // logEntry("[  COMP]: Zlib has reported a data stream error, aborting process.");
      return -1;
    } // if

    if (myCompStream.avail_out < NETWORK_MTU) {
      // send(compressSocket, myCBuffer, (NETWORK_MTU - myCompStream.avail_out), 0);
     addWriteBuffer((const char *) myCBuffer, (NETWORK_MTU - myCompStream.avail_out));
      // cout << "compressTo(" << compressTo << ")" << endl;
      // sendto->write((const char *)to, (NETWORK_MTU - myCompStream.avail_out));
    } // if
  } // while

  myPacketCount++;
  if (myPacketCount > NETWORK_COMPRESSION_REPORTING) {
    //app->writeLog(OPENAPRS_LOG_DEBUG, "Connection::compressBuffer> I(%li), O(%li), %d%% ratio.", myCompStream.total_in, myCompStream.total_out, (int) (((double) myCompStream.total_out / (double) myCompStream.total_in) * 100));
    myPacketCount = 0;
  } // if

  return (compressFromSize - myCompStream.avail_in);
} // Connection::compressBuffer

const int Connection::deCompressBuffer(Bytef *deCompressFrom, const int deCompressFromSize, string &deCompressTo) {
  char lBuffer[NETWORK_MTU + 1024];			// temp compression buffer

  myDecompStream.next_in = deCompressFrom;
  myDecompStream.avail_in = deCompressFromSize;

  // initialize variables
  deCompressTo = "";

  memcpy(lBuffer, deCompressFrom, deCompressFromSize);
  lBuffer[deCompressFromSize] = '\0';
  // cout << "deCompressFrom(" << lBuffer << ")" << endl;

  while((myDecompStream.avail_in) > 0 || (myDecompStream.avail_out < NETWORK_MTU)) {
    myDecompStream.next_out = myCBuffer;
    myDecompStream.avail_out = NETWORK_MTU;
    int zlibCode = inflate(&myDecompStream, Z_SYNC_FLUSH);

    if (zlibCode == Z_STREAM_ERROR || zlibCode == Z_DATA_ERROR) {
      // logEntry("[DECOMP]: Zlib has reported a data stream error, aborting process.");
      return -1;
    } // if

    if (myDecompStream.avail_out < NETWORK_MTU) {
      memcpy(lBuffer, myCBuffer, (NETWORK_MTU - myDecompStream.avail_out));
      lBuffer[NETWORK_MTU - myDecompStream.avail_out] = '\0';

      deCompressTo += lBuffer;

      // cout << "!deCompressTo(" << deCompressTo << ")" << endl;
      // sendto->write((const char *)to, (NETWORK_MTU - decompStream.avail_out));
    } // if
  } // while

  myPacketCount++;
  if (myPacketCount > NETWORK_COMPRESSION_REPORTING) {
    //app->writeLog(OPENAPRS_LOG_DEBUG, "Connection::deCompressBuffer> I(%li), O(%li), %d%% ratio.", myDecompStream.total_in, myDecompStream.total_out, (int) (((double) myDecompStream.total_in / (double) myDecompStream.total_out) * 100));
    myPacketCount = 0;
  } // if

  return (deCompressFromSize - myDecompStream.avail_in);
} // Connection::deCompressBuffer

const int Connection::sendf(const char *fmt, ...) {
  char sendBuffer[MAXBUF + 1] = {0};			// buffer
  va_list va;

  va_start(va, fmt);
  vsnprintf(sendBuffer, sizeof(sendBuffer), fmt, va);
  va_end(va);

  // send compresse sockets to sendPacket instead
  if (getCompress() == true)
    compressBuffer((Bytef *) sendBuffer, strlen(sendBuffer));
  else
    addWriteBuffer(sendBuffer);

  return strlen(sendBuffer);
} // Connection::sendf

/**************************************************************************
 ** Network Class                                                        **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

Network::Network() {
  FD_ZERO(&myReadSet);
  FD_ZERO(&myWriteSet);
  myHighestFd = FD_SETSIZE;

  return;
} // Network::Network

Network::~Network() {
  return;
} // Network::~Network

/************************
 ** Connection Members **
 ************************/

Connection *Network::addConnection(const string addIP, const string addPort, const string addLocalIP, const string addDescription, const bool addBinary) {
  Connection *nConnection;			// pointer to a new connection
  int newFd;					// Fd for new connection
  int sendSize;					// Fd send size
  unsigned int setSendSize;			// size we should set to
  socklen_t optlen;				// option length

  newFd = makeConnection(addIP, addPort, addLocalIP);

  if (newFd == NETWORK_ERROR)
    return NULL;

  // initialize variables
  setSendSize = NETWORK_MTU;

  if (setsockopt(newFd, SOL_SOCKET, SO_RCVBUF, &setSendSize, sizeof(setSendSize)) < 0 ||
      setsockopt(newFd, SOL_SOCKET, SO_SNDBUF, &setSendSize, sizeof(setSendSize)) < 0) {
    //app->writeLog(OPENAPRS_LOG_DEBUG, "Network::addConnection[setsockopt]> %s", hstrerror(h_errno));
    _error = hstrerror(h_errno);
    _errno = h_errno;
    return NULL;
  } // if

  // initialize variables
  sendSize = 0;
  optlen = sizeof(sendSize);

  if (getsockopt(newFd, SOL_SOCKET, SO_SNDBUF, (char *) &sendSize, &optlen) < 0) {
    //app->writeLog(OPENAPRS_LOG_DEBUG, "Network::addConnection[getsockopt]> %s", hstrerror(h_errno));
    _error = hstrerror(h_errno);
    _errno = h_errno;
    return NULL;
  } // if

  // initialize variables
  nConnection = new Connection;
  nConnection->setSendBufferSize(sendSize);
  nConnection->setFd(newFd);
  nConnection->setDescription(addDescription);
  nConnection->setIP(addIP);
  nConnection->setPort(addPort);
  if (addBinary == true)
    SetConnectionBinary(nConnection);

  connectionList.insert(pair<int, Connection *>(newFd, nConnection));

  // initialize variables

  // add Fd to read set
  FD_SET(newFd, &myReadSet);
  FD_SET(newFd, &myWriteSet);
  if (newFd > myHighestFd)
    myHighestFd = newFd;

  return nConnection;
} // Network::addConnection

Connection *Network::addConnection(const int addFd, const string &addDescription) {
  Connection *nConnection;			// a connection

  // initialize variables
  nConnection = new Connection;
  nConnection->setFd(addFd);
  nConnection->setDescription(addDescription);
  UnsetConnectionBinary(nConnection);

  connectionList.insert(pair<int, Connection *>(addFd, nConnection));

  // initialize variables

  // add Fd to read set
  FD_SET(addFd, &myReadSet);
  if (addFd > myHighestFd)
    myHighestFd = addFd;

  return nConnection;
} // Network::addConnection

const bool Network::addConnection(Connection *connection, const string &addDescription) {
  int newFd = connection->getFd();
  map<int, Connection *>::iterator ptr;
  int sendSize;					// Fd send size
  unsigned int setSendSize;			// size we should set to
  socklen_t optlen;				// option length

  assert(connection != NULL);

  ptr = connectionList.find(newFd);
  if (ptr != connectionList.end())
    return false;

  // initialize variables
  setSendSize = NETWORK_MTU;

  if (setsockopt(newFd, SOL_SOCKET, SO_RCVBUF, &setSendSize, sizeof(setSendSize)) < 0 ||
      setsockopt(newFd, SOL_SOCKET, SO_SNDBUF, &setSendSize, sizeof(setSendSize)) < 0) {
    //app->writeLog(OPENAPRS_LOG_DEBUG, "Network::addConnection[setsockopt]> %s", hstrerror(h_errno));
    _error = hstrerror(h_errno);
    _errno = h_errno;
    return false;
  } // if

  // initialize variables
  sendSize = 0;
  optlen = sizeof(sendSize);

  if (getsockopt(newFd, SOL_SOCKET, SO_SNDBUF, (char *) &sendSize, &optlen) < 0) {
    //app->writeLog(OPENAPRS_LOG_DEBUG, "Network::addConnection[getsockopt]> %s", hstrerror(h_errno));
    _error = hstrerror(h_errno);
    _errno = h_errno;
    return false;
  } // if

  connectionList.insert(pair<int, Connection *>(newFd, connection));

  // initialize variables

  // add Fd to read set
  FD_SET(newFd, &myReadSet);
  FD_SET(newFd, &myWriteSet);
  if (newFd > myHighestFd)
    myHighestFd = newFd;

  return true;
} // Network::addConnection

/*
 * Attempts to close and remove a connection
 *
 * Returns:
 *
 * true				Successfully removed connection
 * false			Could not find connection in list
 */
const bool Network::removeConnection(const int removeFd) {
  return removeConnection(removeFd, true);
} // Network::removeConnection

const bool Network::removeConnection(const int removeFd, const bool destroy) {
  Connection *lConnection;			// pointer to a looped to connection
  Connection *tConnection;			// pointer to a target connection
  connectionMapType::iterator connectionPtr;	// pointer to a map

  if ((connectionPtr = connectionList.find(removeFd)) == connectionList.end())
    return false;

  // initialize variables
  tConnection = connectionPtr->second;

  // attempt to close the Fd
  if (tConnection->getFd() != -1 && destroy)
    tConnection->close();

  //app->writeLog(OPENAPRS_LOG_NORMAL, "*** Disconnected from %s (%s)", tConnection->getIP().c_str(), tConnection->getPort().c_str());

  // remove add free the connection
  if (destroy && !tConnection->retain())
    delete tConnection;
  connectionList.erase(connectionPtr);

  FD_CLR(removeFd, &myReadSet);
  FD_CLR(removeFd, &myWriteSet);

  /*
   * 07/01/2003: Loop through our connection list to find our new highest fd.
   */

  // initialize variables
  myHighestFd = FD_SETSIZE;

  for(connectionPtr = connectionList.begin(); 
      connectionPtr != connectionList.end(); connectionPtr++) {

    // initialize variables
    lConnection = connectionPtr->second;

    if (lConnection->getFd() > myHighestFd)
      myHighestFd = lConnection->getFd();
  } // for

  return true;
} // Network::removeConnection

/*
 * Attempts to make a connection.
 *
 * Returns:
 *
 * # > 0			Connected
 * # = 0			No connection
 * # = -1			Close
 */
const int Network::readConnectionPacket(const int readFd, string &readData) {
  connectionMapType::iterator ptr;		// pointer to a map

  ptr = connectionList.find(readFd);
  if (ptr == connectionList.end())
    return NETWORK_ERROR;

  if (ptr->second->readLine(readData) == true)
    return NETWORK_TRUE;

  return NETWORK_FALSE;
} // Network::readConnectionPacket

/*
 * Read binary from Fd.
 *
 * Returns:
 *
 * # > 0			Connected
 * # = 0			No connection
 * # = -1			Close
 */
const int Network::readConnectionBinary(const int readFd, char *readData, unsigned int readLength) {
  connectionMapType::iterator ptr;		// pointer to a map

  ptr = connectionList.find(readFd);
  if (ptr == connectionList.end())
    return NETWORK_ERROR;

  return ptr->second->getPacket(readData, readLength);
} // Network::readConnectionBinary

/*
 * Attempts to make a connection.
 *
 * Returns:
 *
 * # > 0			Connected
 * # = 0			No connection
 * # = -1			Close
 */
const int Network::makeConnection(const string &makeIP, const string &makePort, const string &makeLocalIP) {
  return makeConnection(makeIP, makePort, makeLocalIP, false);
} // Network::makeConnection

const int Network::makeConnection(const string &makeIP, const string &makePort, const string &makeLocalIP,
                                  const bool blocking) {
  int newFd;			// Fd
  string connectTo;			// what host to connect to
  struct sockaddr_in sa;		// target
  struct in_addr in;			// ip address
  struct hostent *hp;			// host
  size_t pos;			// position in string

  pos = makeIP.find(".");
  if (pos == string::npos) {
    in.s_addr = ntohl(strtoul(makeIP.c_str(), NULL, 0));
    connectTo = inet_ntoa(in);
  } // if
  else
    connectTo = makeIP;

  if ((hp = gethostbyname(connectTo.c_str())) == NULL) {
    //app->writeLog(OPENAPRS_LOG_DEBUG, "Network::makeConnection[gethostbyname]> %s", hstrerror(h_errno));
    _error = hstrerror(h_errno);
    _errno = h_errno;
    return NETWORK_ERROR;
  } // if

  bzero(&sa, sizeof(sa));
  bcopy(hp->h_addr, (char *) &sa.sin_addr, hp->h_length);
  sa.sin_family = hp->h_addrtype;
  sa.sin_port = htons((u_short) atoi(makePort.c_str()));

  if ((newFd = socket(hp->h_addrtype, SOCK_STREAM, 0)) < 0) {
    //app->writeLog(OPENAPRS_LOG_DEBUG, "Network::makeConnection[socket]> %s", strerror(errno));
    _error = strerror(errno);
    _errno = errno;
    return NETWORK_ERROR;
  } // if

  /*
   * This section really isn't needed in most cases. However if the user
   * wishes to bind to another interface they have the freedom to before the
   * socket is connected.
   */
  if (makeLocalIP.length() > 0) {
    struct sockaddr_in la;
    struct hostent *vh;

    if ((vh = gethostbyname(makeLocalIP.c_str())) == NULL) {
      //app->writeLog(OPENAPRS_LOG_DEBUG, "Network::makeConnection[gethostbyname]> %s", hstrerror(h_errno));
      _error = hstrerror(h_errno);
      _errno = h_errno;
      return NETWORK_ERROR;
    } // if

    bzero(&la, sizeof(la));
    bcopy(vh->h_addr, (char *) &la.sin_addr, vh->h_length);
    la.sin_family = vh->h_addrtype;
    la.sin_port = 0;

    if (bind(newFd, (struct sockaddr *) & la, sizeof(la)) == -1) {
      close(newFd);
      //app->writeLog(OPENAPRS_LOG_DEBUG, "Network::makeConnection[bind]> %s", strerror(errno));
      _error = strerror(errno);
      _errno = errno;
      return NETWORK_ERROR;
    } // if
  }   // if

  /*
   * Connect the site to the socket and let us go!
   */
  if (connect(newFd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
    close(newFd);
    //app->writeLog(OPENAPRS_LOG_DEBUG, "Network::makeConnection[connect]> %s", strerror(errno));
    _error = strerror(errno);
    _errno = errno;
    return NETWORK_ERROR;
  } // if

  // set non-blocking
  if (!blocking) {
    if (fcntl(newFd, F_SETFL, O_NONBLOCK) == -1) {
      close(newFd);
      //app->writeLog(OPENAPRS_LOG_DEBUG, "Network::makeConnection[fcntl]> %s", strerror(errno));
      _error = strerror(errno);
      _errno = errno;
      return NETWORK_ERROR;
    } // if
  } // if

  // app->writeLog(OPENAPRS_LOG_NORMAL, "Connect[%s] %s:%s", inet_ntoa(*((struct in_addr *) hp->h_addr_list[0])), makeIP.c_str(), makePort.c_str());
  //app->writeLog(OPENAPRS_LOG_NORMAL, "*** Trying %s (%s)", makeIP.c_str(), makePort.c_str());

  return newFd;
} // Network::makeConnection


/*
 * Attempts to open a socket
 *
 * Returns:
 *
 * # > 0			Connected
 * # = 0			No connection
 * # = -1			Close
 */
const int Network::makeSocket(const int makePort, const int maxConnections) {
  return makeSocket(makePort, maxConnections, false);
} // Network::makeSocket

const int Network::makeSocket(const int makePort, const int maxConnections, const bool blocking) {
  SocketType socketInfo;			// socket info
  int newFd;				// Fd number
  int returnResult;				// returned value
  unsigned int opt;				// setsockopt options

  // setup the sock_addr data structure
  socketInfo.sin_family = AF_INET;
  socketInfo.sin_addr.s_addr = INADDR_ANY;
  socketInfo.sin_port = htons(makePort);

  /* create the new socket */
  newFd = socket(AF_INET, SOCK_STREAM, 0);
  if (newFd == -1) {
    //app->writeLog(OPENAPRS_LOG_DEBUG, "Network::makeSocket[socket]> %s", strerror(errno));
    _error = strerror(errno);
    _errno = errno;
    return NETWORK_ERROR;
  } // if

  // allow the ip to be reused
  opt = 1;

  if (setsockopt(newFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
    close(newFd);
    //app->writeLog(OPENAPRS_LOG_DEBUG, "Network::makeSocket[setsockopt]> %s", strerror(errno));
    _error = strerror(errno);
    _errno = errno;
    return NETWORK_ERROR;
  } // if

  // bind the socket
  returnResult = bind(newFd, (struct sockaddr *) &socketInfo, sizeof(socketInfo));
  if (returnResult == -1) {
    //app->writeLog(OPENAPRS_LOG_DEBUG, "Network::makeSocket[bind]> %s", strerror(errno));
    _error = strerror(errno);
    _errno = errno;
    return NETWORK_ERROR;
  } // if

  // set up the socket for listening
  returnResult = listen(newFd, maxConnections);
  if (returnResult == -1) {
    //app->writeLog(OPENAPRS_LOG_DEBUG, "Network::makeSocket[listen]> %s", strerror(errno));
    _error = strerror(errno);
    _errno = errno;
    return NETWORK_ERROR;
  } // if

  // set non-blocking
  if (!blocking) {
    if (fcntl(newFd, F_SETFL, O_NONBLOCK) == -1) {
      close(newFd);
      //app->writeLog(OPENAPRS_LOG_DEBUG, "Network::makeSocket[fcntl]> %s", strerror(errno));
      _error = strerror(errno);
      _errno = errno;
      return NETWORK_ERROR;
    } // if
  } // if

  // return the new socket descriptor
  return newFd;
} // Network::makeSocket


/*
 * Attempts to accept a connection.
 *
 * Returns:
 *
 * # > 0			Connected
 * # = 0			No connection
 * # = -1			Close
 */
const int Network::acceptConnection(const int acceptFd, const bool acceptBinary) {
  Connection *aConnection;		// new connection
  SocketType socketInfo;		// socket information
  int packetLength;			// info length
  int newFd;				// socket number
  int sendSize;				// socket send size
  socklen_t optlen;			// option length

  readConnections();

  // accept the connections if there are any
  packetLength = sizeof(socketInfo);
  newFd = accept(acceptFd, (struct sockaddr *) &socketInfo, (socklen_t *) &packetLength);

  if (newFd == -1) {
//    debug(HERE, "Network::acceptConnection[accept]():  errno(%d) errmsg(%s)\n", errno, strerror(errno));

    switch(errno) {
      // The descriptor is invalid.
      case EBADF:
      // The per-process descriptor table is full.
      case EMFILE:
      // The descriptor references a file, not a socket.
      case ENOTSOCK:
      // listen(2) has not been called on the socket descriptor.
      case EINVAL:
      // The addr parameter is not in a writable part of the user address space.
      case EFAULT:
      // A connection arrived, but it was closed while waiting on the listen queue.
      case ECONNABORTED:
        // close and move on
        return NETWORK_ERROR;
        break;
      // The accept() operation was interrupted.
      case EINTR:
      // The socket is marked non-blocking and no connections are present to be accepted.
      case EWOULDBLOCK:
        // no connections ignore and skip
        return NETWORK_FALSE;
        break;
    } // switch
  } // if

  // initialize variables
  sendSize = 0;
  optlen = sizeof(sendSize);

  if (getsockopt(newFd, SOL_SOCKET, SO_SNDBUF, (char *) &sendSize, &optlen) < 0) {
    //app->writeLog(OPENAPRS_LOG_DEBUG, "Network::acceptConnection[getsockopt]> %s", hstrerror(h_errno));
    _error = hstrerror(h_errno);
    _errno = h_errno;
    return NETWORK_ERROR;
  } // if

  // initialize variables
  aConnection = new Connection;
  aConnection->setFd(newFd);
  aConnection->setSendBufferSize(sendSize);
  aConnection->setDescription("Accepted Connection");
  if (acceptBinary == true)
    SetConnectionBinary(aConnection);

  connectionList.insert(pair<int, Connection *>(newFd, aConnection));

  // add socket to read set
  FD_SET(newFd, &myReadSet);
  FD_SET(newFd, &myWriteSet);

  // we have a successul connection
  // return new socket
  return newFd;
}

const int Network::getConnectionPortNumber(const int myFd) {
  int length;				// length
  int result;				// results
  struct sockaddr_in address;		// address

  length = sizeof(address);
  result = getsockname(myFd, (struct sockaddr *) &address, (socklen_t *) &length);

  if (result == -1) {
    //perror(NULL);
    return NETWORK_ERROR;
  } // if

  return ntohs(address.sin_port);
} // Network::getConnectionPortNumber

const unsigned long Network::getConnectionNetworkNumber(const int myFd) {
  int length;				// length
  int result;				// results
  struct sockaddr_in address;		// address

  length = sizeof(address);
  result = getsockname(myFd, (struct sockaddr *) &address, (socklen_t *) &length);

  if (result == -1) {
    //perror(NULL);
    return NETWORK_FALSE;
  } // if

  return ntohl(address.sin_addr.s_addr);
} // Network::getConnectionNetworkNumber

const unsigned long Network::getConnectionPeerNetworkNumber(const int myFd) {
  int length;				// length
  int result;				// results
  struct sockaddr_in address;		// address

  length = sizeof(address);
  result = getpeername(myFd, (struct sockaddr *) &address, (socklen_t *) &length);

  if (result == -1) {
    //perror(NULL);
    return NETWORK_FALSE;
  } // if

  return ntohl(address.sin_addr.s_addr);
} // Network::getConnectionPeerNetworkNumber

const int Network::readConnections() {
  char buf[NETWORK_MTU];			// temp buffer
  fd_set readSet;				// read set
  int packetLength;				// length
  int returnResult;				// return values
  list<int> removeList;				// remove list
  connectionMapType::iterator ptr;		// pointer to a map
  string tempBuf;				// temporary buffer

  // initialize variables
  timeval timeout = {0, 50000};
  readSet = myReadSet;

  returnResult = select(FD_SETSIZE, &readSet, NULL, NULL, &timeout);

  if (returnResult == -1) {
    //app->writeLog(OPENAPRS_LOG_DEBUG, "Network::readConnections[select]> %s", strerror(errno));
    _error = strerror(errno);
    _errno = errno;

    //  If we got an error from select let's figure out why.
    switch(errno) {
      case EBADF:
      // The specified time limit is invalid.  One of its components is negative or too large.
      case EINVAL:
      // A signal was delivered before the time limit expired and before any of the selected events occurred.
      case EINTR:
      // One of the descriptor sets specified an invalid descriptor.
      default:
        return NETWORK_FALSE;
        break;
    } // switch

    // this shouldn't happen
    assert(false);

    // error maybe we should ignore
    return NETWORK_FALSE;
  } // if

  // loop through connections
  ptr = connectionList.begin();
  while(ptr != connectionList.end()) {
    // handle reads
    if (FD_ISSET(ptr->second->getFd(), &readSet)) {
      packetLength = read(ptr->second->getFd(), buf, NETWORK_MTU);

      // is there anything to read?
      if (packetLength == -1) {
        //app->writeLog(OPENAPRS_LOG_DEBUG, "Network::readConnections[read]> %s", strerror(errno));
        _error = strerror(errno);
        _errno = errno;

        //  If we got an error from select let's figure out why.
        switch(errno) {
          /******************
           ** Fatal Errors **
           ******************/
          // The receive buffer pointer(s) point outside the pro-
          // cess's address space.
          case EFAULT:
          // The socket is associated with a connection-oriented
          // protocol and has not been connected (see connect(2)
          // and accept(2)).
          case ENOTCONN:
          // The argument s is an invalid descriptor.
          case EBADF:
            // this list of cases should not be added to the keep
            // list, we want to remove them
            // don't remove the connection until the buffer is cleared.
            if (ptr->second->size() < 1)
              removeList.push_back(ptr->second->getFd());
            ptr++;
            continue;
            break;
          /**********************
           ** Non-Fatal Errors **
           **********************/
          // The receive was interrupted by delivery of a signal
          // before any data were available.
          case EINTR:
          // The socket is marked non-blocking, and the receive
          // operation would block, or a receive timeout had been
          // set, and the timeout expired before data were received.
          case EAGAIN:
          default:
            ptr++;
            continue;
            break;
        } // switch
      } // if

      // client is disconnected
      if (packetLength == 0 && ptr->second->size() < 1) {
        removeList.push_back(ptr->second->getFd());
        ptr++;
        continue;
      } // if

      // debug(HERE, "Network::readConnections[recv] packetLength(%d)  sizeof(buf)(%d)\n", packetLength, sizeof(buf));

      if (ptr->second->getCompress() == true) {
        if (ptr->second->deCompressBuffer((Bytef *) buf, packetLength, tempBuf) == -1) {
          removeList.push_back(ptr->second->getFd());
          ptr++;
          continue;
        } // if

        ptr->second->addLine(tempBuf);
      } // if
      else {
        buf[packetLength] = '\0';
        if (IsConnectionBinary(ptr->second) == false)
          ptr->second->addLine(buf);
        else
          ptr->second->addPacket(buf, packetLength);
      } // if
    } // if

    // increment to next connection
    ptr++;
  } // while

  // remove closed connections
  while(!removeList.empty()) {
    ptr = connectionList.find(removeList.front());
    if (ptr != connectionList.end()) {
      FD_CLR(ptr->second->getFd(), &myReadSet);
      FD_CLR(ptr->second->getFd(), &myWriteSet);
      ptr->second->close();

      if (!ptr->second->retain())
        delete ptr->second;

      connectionList.erase(ptr);
    } // if

    // remove from list
    removeList.pop_front();
  } // while

  // return an ignore value
  // should never really get here
  return NETWORK_TRUE;
} // Network::readConnections

const int Network::writeConnections() {
  bool writeError;				// write error?
  fd_set writeSet;				// write set
  int packetLength;				// length
  int returnResult;				// result returned
  list<int> removeList;				// remove list
  connectionMapType::iterator ptr;		// pointer to a map
  string writeOut;				// write out

  // initialize variables
  timeval timeout = {0, 50000};
  writeSet = myWriteSet;

  returnResult = select(FD_SETSIZE, NULL, &writeSet, NULL, &timeout);

  if (returnResult == -1) {
    //app->writeLog(OPENAPRS_LOG_DEBUG, "Network::writeConnections[select]> %s", strerror(errno));
    _error = strerror(errno);
    _errno = errno;

    //  If we got an error from select let's figure out why.
    switch(errno) {
      case EBADF:
      // The specified time limit is invalid.  One of its components is negative or too large.
      case EINVAL:
      // A signal was delivered before the time limit expired and before any of the selected events occurred.
      case EINTR:
      // One of the descriptor sets specified an invalid descriptor.
      default:
        return NETWORK_FALSE;
        break;
    } // switch

    // this shouldn't happen
    assert(false);

    // error maybe we should ignore
    return NETWORK_FALSE;
  } // if

  // loop through connections
  for(ptr = connectionList.begin(); ptr != connectionList.end(); ptr++) {
    // don't try to send until timeout is up (ENOBUFS)
    if (ptr->second->getWriteNextTS() > time(NULL))
      continue;

    // handle reads
    if (FD_ISSET(ptr->second->getFd(), &writeSet)) {
      // initialize variables
      writeError = false;

      while(ptr->second->getWriteBuffer().size() > 0 && writeError == false) {
        packetLength = send(ptr->second->getFd(), ptr->second->getWriteBuffer().c_str(), OPENAPRS_MIN((unsigned) ptr->second->getWriteBuffer().size(), (unsigned) ptr->second->getSendBufferSize()), 0);

        // did we send?
        if (packetLength == -1) {
          //app->writeLog(OPENAPRS_LOG_DEBUG, "Network::readConnections[send]> %s", strerror(errno));
          _error = strerror(errno);
          _errno = errno;

          // if the compression failed close the socket
          if (ptr->second->getCompress() == true) {
            removeList.push_back(ptr->second->getFd());
            writeError = true;
            continue;
          } // if

          //  If we got an error from select let's figure out why.
          switch(errno) {
            /******************
             ** Fatal Errors **
             ******************/
            // The argument s is not a socket.
            case ENOTSOCK:
            // The destination address is a broadcast address, and
            // SO_BROADCAST has not been set on the socket.
            case EACCES:
            // The receive buffer pointer(s) point outside the pro-
            // cess's address space.
            case EFAULT:
            // The socket is associated with a connection-oriented
            // protocol and has not been connected (see connect(2)
            // and accept(2)).
            case ENOTCONN:
            // The argument s is an invalid descriptor.
            case EBADF:
              // this list of cases should not be added to the keep
              // list, we want to remove them
              removeList.push_back(ptr->second->getFd());
              break;
            /**********************
             ** Non-Fatal Errors **
             **********************/
            // The output queue for a network interface was full.
            // This generally indicates that the interface has
            // stopped sending, but may be caused by transient con-
            // gestion.
            case ENOBUFS:
              // make sure when this happens that we only
              // attempt to write again to this socket
              // after one second passes, prevents cpu hogging
              // The socket is marked non-blocking, and the receive
              // operation would block, or a receive timeout had been
              // set, and the timeout expired before data were received.
              ptr->second->setWriteNextTS(time(NULL) + 5);
            case EAGAIN:
            default:
              break;
          } // switch

          // make sure we exit the loop
          // an error occured
          writeError = true;
          continue;
        } // if

        ptr->second->eraseWriteBuffer(0, packetLength);

        // this'll flood the heck outta the
        // log file
        // debug(HERE, "Network::writeConnections[send] packetLength(%d)\n", packetLength);
      } // while
    } // if
  } // for

  // remove closed connections
  while(!removeList.empty()) {
    ptr = connectionList.find(removeList.front());
    if (ptr != connectionList.end()) {
      FD_CLR(ptr->second->getFd(), &myReadSet);
      FD_CLR(ptr->second->getFd(), &myWriteSet);
      ptr->second->close();

      if (!ptr->second->retain())
        delete ptr->second;

      connectionList.erase(ptr);
    } // if

    // remove from list
    removeList.pop_front();
  } // while

  // return an ignore value
  // should never really get here
  return NETWORK_TRUE;
} // Network::writeConnections

const int Network::sendConnectionPacket(const int sendFd, const string &sendMe, const unsigned int sendLength) {
  connectionMapType::iterator ptr;			// pointer to a map
  int returnResult;					// return result
  int compressLength;					// compress length

  ptr = connectionList.find(sendFd);
  if (ptr == connectionList.end())
    return NETWORK_ERROR;

  if (ptr->second->getCompress() == true) {
    if ((compressLength = ptr->second->compressBuffer((Bytef *) sendMe.c_str(), sendLength)) == -1) {
      ptr->second->close();

      if (!ptr->second->retain())
        delete ptr->second;

      connectionList.erase(ptr);
      return NETWORK_ERROR;
    } // if

    // returnResult = send(ptr->first, returnCompressed.c_str(), returnCompressed.length(), 0);
  } // if
  else
    returnResult = send(ptr->first, sendMe.c_str(), sendLength, 0);

  if (returnResult == -1) {
    //app->writeLog(OPENAPRS_LOG_DEBUG, "Network::sendPacket[send]> %s", strerror(errno));
    _error = strerror(errno);
    _errno = errno;
    removeConnection(ptr->first);
  } // if

  return returnResult;
} // Network::sendConnectionPacket

const int Network::sendConnectionF(const int sendFd, const char *sendFormat, ...) {
  Connection *tConnection;				// pointer to a target connection
  connectionMapType::iterator connectionPtr;		// pointer to a map
  char sendBuffer[MAXBUF + 1] = {0};			// buffer
  va_list va;

  if ((connectionPtr = connectionList.find(sendFd)) == connectionList.end())
    return NETWORK_ERROR;

  // initialize variables
  tConnection = connectionPtr->second;

  va_start(va, sendFormat);
  vsnprintf(sendBuffer, sizeof(sendBuffer), sendFormat, va);
  va_end(va);

  // send compresse sockets to sendPacket instead
  if (tConnection->getCompress() == true)
    tConnection->compressBuffer((Bytef *) sendBuffer, strlen(sendBuffer));
  else
    tConnection->addWriteBuffer(sendBuffer);

  return strlen(sendBuffer);
} // Network::sendConnectionF

}
