/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, Internet APRS MySQL Injector                               **
 ** Copyright (C) 1999 Gregory A. Carter                                 **
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
 $Id: Decay.cpp,v 1.2 2005/12/13 21:07:03 omni Exp $
 **************************************************************************/

#include <list>
#include <new>
#include <string>
#include <cstdarg>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

#include <dlfcn.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#include "APRS.h"
#include "Decay.h"
#include "Create.h"
#include "Log.h"
#include "Server.h"
#include "Send.h"

#include "config.h"
#include "openaprs.h"
#include "openaprs_string.h"
#include "md5wrapper.h"

namespace openaprs {

using std::string;
using std::list;
using std::ofstream;
using std::ostream;
using std::stringstream;
using std::ios;
using std::endl;
using std::cout;

/**************************************************************************
 ** Decay Class                                                          **
 **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  Decay::Decay() {
  } // Decay::Decay

  Decay::~Decay() {
    clear();
  } // Decay::~Decay

  /***************
   ** Variables **
   ***************/

  /*********************
   ** Message Members **
   *********************/

  const bool Decay::add(const string &source, const string &name,
                  const string &message, const time_t rate, const time_t max,
                  string &id) {
    DecayMe *d;
    md5wrapper md5;
    stringstream s;

    id="";

    if (source.length() < 1)
      return false;

    if (message.length() < 1)
      return false;

    if (rate <= 0)
      return false;

    if (max <= 0)
      return false;

    // rate*2 must be less than max
    if (rate*2 > max)
      return false;

    s.str();
    s << time(NULL) << source << message << rate;

    d = new DecayMe;

    d->rate = rate;
    d->max = max;
    d->source = source;
    d->count = 0;
    d->broadcast_ts = time(NULL);
    d->message = message;
    d->name = name;
    d->create_ts = time(NULL);
    d->id = md5.getHashFromString(s.str());

    id = d->id;

    _slogf(OPENAPRS_LOG_APRS, "decay{ADD}: %s resending in %d:%02d seconds from %s",
          d->name.c_str(),
          (d->rate/60),
          (d->rate%60),
          d->source.c_str());

    _decayList.push_back(d);

    return true;
  } // Decay::add

  const unsigned int Decay::remove(const string &id) {
    DecayMe *d;
    decayListType newList;
    int i;
    time_t now=time(NULL);

    if (id.length() < 1)
      return 0;

    for(i=0; !_decayList.empty();) {
      d = _decayList.front();

      if (d->id != id)
        newList.push_back(d);
      else {
        _slogf(OPENAPRS_LOG_APRS, "decay{REMOVE}: %s after %d:%02d seconds from %s",
                    d->name.c_str(),
                    ((now-d->create_ts)/60),
                    ((now-d->create_ts)%60),
                    d->source.c_str());
        delete d;
        i++;
      } // else

      _decayList.pop_front();
    } // while

    _decayList = newList;

    return i;
  } // Decay::Remove

  const int Decay::next(decayStringsType &decayStrings) {
    DecayMe *d;
    decayListType newList;
    int i;
    time_t now=time(NULL);

    for(i=0; !_decayList.empty();) {
      d = _decayList.front();

      if ((d->broadcast_ts+d->rate) < now) {
        decayStrings.push_back(d->message);
        _slogf(OPENAPRS_LOG_APRS, "decay{RETRY}: #%d) %s after %d:%02d seconds, next in %d:%02d seconds from %s",
                    d->count+1,
                    d->name.c_str(),
                    (d->rate/60),
                    (d->rate%60),
                    (d->rate*2/60),
                    ((d->rate*2)%60),
                    d->source.c_str());
        d->broadcast_ts = now;
        d->rate = d->rate*2;
        d->count++;
        i++;
      } // if

      if (d->rate <= d->max)
        newList.push_back(d);
      else {
        _slogf(OPENAPRS_LOG_APRS, "decay{DONE}: %s after %d:%02d seconds from %s",
                    d->name.c_str(),
                    ((now-d->create_ts)/60),
                    ((now-d->create_ts)%60),
                    d->source.c_str());
         delete d;
       } // else

      _decayList.pop_front();
    } // while

    _decayList = newList;

    return i;
  } // Decay::next

  const Decay::decayListSizeType Decay::clear() {
    decayListSizeType ret = _decayList.size();

    while(!_decayList.empty()) {
      delete _decayList.front();
      _decayList.pop_front();
    } // while

    return ret;
  } // Decay::clear
} // namespace openaprs
