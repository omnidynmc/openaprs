/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, mySQL APRS Injector                                        **
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
 $Id: Reply.h,v 1.8 2003/09/04 00:22:00 omni Exp $
 **************************************************************************/

#ifndef __MODULE_REPLYCLASS_H
#define __MODULE_REPLYCLASS_H

#include "noCaseCompare.h"

#include "openframe/OFLock.h"

namespace dcc {

  using openframe::OFLock;
  using openaprs::noCaseCompare;
  using namespace std;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class ReplyMessage {
  public:
    string message;
    string name;
    unsigned int number;
}; // class ReplyMessage

class Reply : public OFLock {
  public:
    Reply();
    virtual ~Reply();

    /**********************
     ** Type Definitions **
     **********************/
    typedef map<string, ReplyMessage, noCaseCompare> replyMapType;

    // public members
    const bool add(const unsigned int, const string &, const string &);
    const bool remove(const string &);
    const unsigned int clear();
    const bool find(const string &, string &);
    const bool findByNumeric(const unsigned int, string &);

  protected:
    const bool _add(const unsigned int, const string &, const string &);
    const bool _remove(const string &);
    const unsigned int _clear();
    const bool _find(const string &, string &);
    const bool _findByNumeric(const unsigned int, string &);

  private:
    replyMapType replyMap;

}; // classReply

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

extern "C" {
} // extern

}
#endif
