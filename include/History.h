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
 **************************************************************************/

#ifndef __LIBHANDLER_HISTORY_H
#define __LIBHANDLER_HISTORY_H

#include <map>
#include <string>
#include <list>
#include <ctime>

#include "APRS.h"
#include "Command.h"
#include "OFLock.h"

namespace openaprs {

using std::string;
using std::map;
using std::list;
using std::pair;
using namespace aprs;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class HistoryPacket {
  public:
    HistoryPacket() {}
    ~HistoryPacket() {}

    double latitude;
    double longitude;
    time_t timestamp;
    string info;
    string id;
};

class History : public OFLock {
  public:
    History();
    ~History();

    typedef list<APRS *> packetListType;
    typedef map<string, packetListType> historyMapType;
    typedef list<historyMapType::iterator> expireListType;

    const bool add(APRS *);
    const bool remove(const string &);
    const bool exists(const string &, const string &);
    const bool position(APRS *);
    const unsigned int expire(const time_t, const unsigned int);
    const unsigned int clear();
    const unsigned int packets() {
      unsigned int num;

      Lock();
      num = numPackets;
      Unlock();

      return num;
    } // packets
    const unsigned int size() {
      unsigned int num;

      Lock();
      num = historyMap.size();
      Unlock();

      return num;
    } // size

  protected:
    const bool _add(APRS *);
    const bool _remove(const string &);
    const bool _exists(const string &, const string &);
    const bool _position(APRS *);
    const unsigned int _expire(const time_t, const unsigned int);
    const unsigned int _clear();

  private:
    historyMapType historyMap;
    unsigned int numPackets;
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

}
#endif
