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
 $Id: Fields.h,v 1.8 2003/09/04 00:22:00 omni Exp $
 **************************************************************************/

#ifndef __MODULE_FIELDSCLASS_H
#define __MODULE_FIELDSCLASS_H

#include "noCaseCompare.h"

namespace dcc {
  using openaprs::noCaseCompare;
  using namespace std;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class Field {
  public:
    string token;
    string name;
    string event;
    string descrip;
}; // class Field

class Fields {
  public:
    Fields();
    virtual ~Fields();

    /**********************
     ** Type Definitions **
     **********************/
    typedef map<string, Field, noCaseCompare> fieldMapType;

    /*************
     ** Members **
     *************/
    const bool add(const string &, const string &, const string &, const string &);
    const bool add(const string &, const string &, const string &);
    const bool remove(const string &);
    const unsigned int clear();
    const bool find(const string &, string &);
    const bool findByName(const string &, string &);
    const bool findByName(const string &, string &, const string &);
    const bool findByEvent(const string &, string &);

  private:
    fieldMapType fieldMap;
    fieldMapType fieldEventMap;

}; // class Fields

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
