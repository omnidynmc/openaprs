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
 $Id: Decay.h,v 1.1 2005/11/21 18:16:02 omni Exp $
 **************************************************************************/

#ifndef __OPENAPRS_DECAY_H
#define __OPENAPRS_DECAY_H

#include <string>
#include <map>

#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "OpenAPRS_Abstract.h"

namespace openaprs {

using std::string;
using std::map;
using std::list;
using std::pair;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class DecayMe {
  public:
    DecayMe() { };
    ~DecayMe() { };

    string id;
    string source;
    string message;
    string name;
    time_t create_ts;
    time_t broadcast_ts;
    time_t rate;
    time_t max;
    unsigned int count;
};

class Decay : public OpenAPRS_Abstract {
  public:
    // ### Type Definitions ###
    typedef list<DecayMe *> decayListType;
    typedef decayListType::size_type decayListSizeType;
    typedef list<string> decayStringsType;

    Decay();				// constructor
    virtual ~Decay();			// destructor

    // ### Variables ###

    // ### Members ###
    const bool add(const string &,
                   const string &,
                   const string &,
                   const time_t,
                   const time_t,
                   string &);
    const unsigned int remove(const string &);
    const int next(decayStringsType &);
    const decayListSizeType size() const { return _decayList.size(); }
    const decayListSizeType clear();

    /***************
     ** Variables **
     ***************/
  public:
  protected:
  private:
    decayListType _decayList;
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

}
#endif
