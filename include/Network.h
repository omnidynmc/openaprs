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
 $Id: Network.h,v 1.3 2005/12/04 21:59:29 omni Exp $
 **************************************************************************/

#ifndef __NETWORK_H
#define __NETWORK_H

#include <fstream>
#include <string>
#include <queue>
#include <cstdlib>
#include <cctype>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <list>
#include <queue>
#include <map>
#include <new>

#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "OpenAPRS_Abstract.h"
#include "LineBuffer.h"
#include "Flag.h"

#include "config.h"
#include "zlib.h"

namespace openaprs {
  using std::string;
  using std::list;
  using std::queue;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/


// #define NETWORK_MTU 1500
#define NETWORK_MTU 61440

#define NETWORK_COMPRESSION_REPORTING			10000

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class Network_Exception : public OpenAPRSAbstract_Exception {
  public:
    Network_Exception(const string message) throw() : OpenAPRSAbstract_Exception(message) {
    } // Network_Exception

  private:
}; // class Network_Exception

class Binary {
  public:
    /********************
     ** Packet Members **
     ********************/
    const unsigned int getPacket(char *setMe, const unsigned int setSize) {
      memcpy(setMe, myPacket, setSize);

      return myLength;
    } // getPacket

    void setPacket(const char *setMe, const unsigned int setSize) {
      setLength(setSize);
      memcpy(myPacket, setMe, sizeof(myPacket));
    } // setPacket

    const unsigned int getLenth() const
      { return myLength; }

    void setLength(const unsigned int setMe)
      { myLength = setMe; }

  private:
    char myPacket[8192];		// packet
    unsigned int myLength;		// packet length
  protected:
}; // Binary

class Connection : public Flag {
  public:
    Connection();
    virtual ~Connection();

    /**********************
     ** Type Definitions **
     **********************/

    typedef queue<string> outputQueueType;
    typedef Flag::flagType flagType;

    /***************
     ** Variables **
     ***************/

    const static flagType FLAG_ZLIB;
    const static flagType FLAG_BINARY;

    /**********************
     ** Compress Members **
     **********************/

    const bool getCompress() const
      { return myCompress; }

    void setCompress(const bool setMe) {
      if (setMe == true) {
        if (myCompress == false) {
          myDecompStream.zalloc = (alloc_func) 0;
          myDecompStream.zfree = (free_func) 0;
          myDecompStream.opaque = (voidpf) 0;

          myCompStream.zalloc = (alloc_func) 0;
          myCompStream.zfree = (free_func) 0;
          myCompStream.opaque = (voidpf) 0;

          inflateInit(&myDecompStream);
          deflateInit2(&myCompStream, Z_BEST_COMPRESSION, Z_DEFLATED, 15, 9, Z_DEFAULT_STRATEGY);
        } // if
      } // if
      else {
        if (myCompress == true) {
          inflateEnd(&myDecompStream);
          deflateEnd(&myCompStream);
        } // if
      } // else

      myCompress = setMe;
    } // setCompress

    const int compressBuffer(Bytef *, const int);
    const int deCompressBuffer(Bytef *, const int, string &);

    /*************************
     ** Description Members **
     *************************/

    const string &getDescription() const
      { return myDescription; }

    void setDescription(const string &setMe)
      { myDescription = setMe; }

    /*******************
     ** Error Members **
     *******************/

    const string &getError() const
      { return myError; }

    void setError(const string &setMe)
      { myError = setMe; }

    /****************
     ** Fd Members **
     ****************/

    const int getFd() const
      { return myFd; }

    void setFd(const int setMe)
      { myFd = setMe; }

    /**************************
     ** Output Queue Members **
     **************************/

    const bool outputQueueEmpty() const
      { return myOutputQueue.empty(); }

    void outputQueuePop()
      { myOutputQueue.pop(); }

    void outputQueuePush(const string pushMe)
      { myOutputQueue.push(pushMe); }

    outputQueueType::value_type &outputQueueFront()
      { return myOutputQueue.front(); }

    /****************
     ** IP Members **
     ****************/

    const string &getIP() const
      { return myIP; }

    void setIP(const string &setMe)
      { myIP = setMe; }

    /******************
     ** Line Members **
     ******************/

    void addLine(const string &addMe)
      { aLine.add(addMe); }

    void clearLine()
      { aLine.flush(); }

    void flushLine(const string &addMe)
      { aLine.flush(); }

    bool readLine(string &returnMe)
      { return aLine.readLine(returnMe); }

    const unsigned int dumpBuffer(string &setMe)
      { return aLine.dumpBuffer(setMe); }

    const unsigned int size()
      { return aLine.size(); }

    /********************
     ** Packet Members **
     ********************/

    void addPacket(const char *addMe, unsigned int addLength) {
      Binary aBinary;			// binary packet

      // initialize variables
      aBinary.setPacket(addMe, addLength);
      aBinary.setLength(addLength);

      binaryList.push_back(aBinary);
      return;
    } // addPacket

    const unsigned int getPacket(char *setMe, unsigned int setLength) {
      unsigned int packetLength;		// packet length

      // initialize variables
      packetLength = 0;

      if (binaryList.empty())
        return packetLength;

      packetLength = binaryList.front().getPacket(setMe, setLength);
      binaryList.pop_front();

      return packetLength;
    } // getPacket

    /******************
     ** Port Members **
     ******************/

    const string &getPort() const
      { return myPort; }

    void setPort(const string &setMe)
      { myPort = setMe; }

    /***********************
     ** Remote IP Members **
     ***********************/

    const string &getRemoteIP() const
      { return myRemoteIP; }

    void setRemoteIP(const string &setMe)
      { myRemoteIP = setMe; }

    /*************************
     ** Remote Port Members **
     *************************/

    const string &getRemotePort() const
      { return myRemotePort; }

    void setRemotePort(const string &setMe)
      { myRemotePort = setMe; }

    /******************************
     ** Send Buffer Size Members **
     ******************************/

    const int getSendBufferSize() const
      { return mySendBufferSize; }

    void setSendBufferSize(const int setMe)
      { mySendBufferSize = setMe; }

    /**************************
     ** Write Buffer Members **
     **************************/

    const string &getWriteBuffer() const
      { return myWriteBuffer; }

    void setWriteBuffer(const string &setMe)
      { myWriteBuffer = setMe; }

    void addWriteBuffer(const string &addMe)
      { myWriteBuffer += addMe; }

    void addWriteBuffer(const char *addMe, const string::size_type addLength)
      { myWriteBuffer.append(addMe, addLength); }

    const string &eraseWriteBuffer(string::size_type eraseFrom, string::size_type eraseTo)
      { return myWriteBuffer.erase(eraseFrom, eraseTo); }

    /***************************
     ** Write Next TS Members **
     ***************************/

    const int getWriteNextTS() const
      { return myWriteNextTS; }

    void setWriteNextTS(const time_t setMe)
      { myWriteNextTS = setMe; }

    const int blocking(const bool);
    void close() {
      if (myFd > 0) {
        ::close(myFd);
        myFd = 0;
      } // if

      _closed = true;
    } // close

    const int sendf(const char *, ...);
    const bool closed() const { return _closed; }
    const bool retain() const { return _retain; }
    void retain(const bool retain) { _retain = retain; }

  protected:
  private:
    /***************
     ** Variables **
     ***************/

    LineBuffer aLine;				// line buffer
    bool myCompress;				// compress link?
    bool _closed;				// is connection closed?
    bool _retain;				// should we retain?
    Bytef myCBuffer[NETWORK_MTU + 1024];	// my compression buffer
    int myFd;					// my fd
    int mySendBufferSize;			// send buffer size
    list<Binary> binaryList;			// binary list
    outputQueueType myOutputQueue;		// output queue
    string myDescription;			// my description of the connection
    string myError;				// my error
    string myIP;				// my ip address
    string myPort;				// my local port
    string myRemoteIP;				// my remote ip address
    string myRemotePort;			// my remote port
    string myWriteBuffer;			// write buffer
    time_t myWriteNextTS;			// next time I'll try to write
    unsigned int myPacketCount;			// number of packets sent
    z_stream myDecompStream;			// decompress stream
    z_stream myCompStream;			// compress stream
};

class Network {
  public:
    Network();
    virtual ~Network();

    /**********************
     ** Type Definitions **
     **********************/

    typedef map<int, Connection *> connectionMapType;

    /************************
     ** Connection Members **
     ************************/

    const bool removeConnection(const int);
    const bool removeConnection(const int, const bool);

    Connection *addConnection(const string, const string, const string, const string, const bool);
    Connection *addConnection(const int, const string &);
    const bool addConnection(Connection *, const string &);
    const string &getError() const { return _error; }
    const int getErrno() const { return _errno; }
    const int makeConnection(const string &, const string &, const string &);
    const int makeConnection(const string &, const string &, const string &, const bool);
    const int makeSocket(const int, const int);
    const int makeSocket(const int, const int, const bool);
    const int getConnectionPortNumber(int);
    const unsigned long getConnectionNetworkNumber(int);
    const unsigned long int getConnectionPeerNetworkNumber(int);
    const int acceptConnection(int, const bool);
    const int readConnections();
    const int writeConnections();
    const int readConnectionPacket(const int, string &);
    const int readConnectionBinary(const int, char *, unsigned int);
    const int sendConnectionPacket(const int, const string &, unsigned int);
    const int sendConnectionF(const int, const char *, ...);
    const int listConnections(connectionMapType &listMe) {
      connectionMapType::iterator ptr;
      unsigned int i;

      // initialize variables
      i = 0;

      for(ptr = connectionList.begin(); ptr != connectionList.end(); ptr++) {
        listMe[ptr->first] = ptr->second;
        i++;
      } // for

      return i;
    }  // Network::listConnections

    Connection *findConnection(const int findMe) {
      connectionMapType::iterator ptr;		// pointer to a map

      ptr = connectionList.find(findMe);
      if (ptr == connectionList.end())
        return NULL;

      return ptr->second;
    } // Network::findConnection

    int setConnectionCompress(const int compressSocket, const bool setMe) {
      connectionMapType::iterator ptr;           // pointer to a map
 
      ptr = connectionList.find(compressSocket);
      if (ptr == connectionList.end())
        return -1;

      ptr->second->setCompress(setMe);

      return 1;
    } // setCompress

  private:

  protected:
    connectionMapType connectionList;			// connection list
    connectionMapType listenList;			// listning list
    fd_set myReadSet;					// my read set
    fd_set myWriteSet;					// my read set
    int myHighestFd;					// Highest fd.
    string _error;
    int _errno;
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

#define NETWORK_TRUE		1
#define NETWORK_FALSE		0
#define NETWORK_ERROR		-1

#define IsConnectionZLIB(x)			x->findFlag(Connection::FLAG_ZLIB)
#define IsConnectionBinary(x)			x->findFlag(Connection::FLAG_BINARY)
#define SetConnectionZLIB(x)			x->addFlag(Connection::FLAG_ZLIB)
#define SetConnectionBinary(x)			x->addFlag(Connection::FLAG_BINARY)
#define UnsetConnectionZLIB(x)			x->removeFlag(Connection::FLAG_ZLIB)
#define UnsetConnectionBinary(x)		x->removeFlag(Connection::FLAG_BINARY)

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

typedef sockaddr_in SocketType;

}
#endif
