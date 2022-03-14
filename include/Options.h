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
 $Id: Options.h,v 1.1 2005/11/21 18:16:03 omni Exp $
 **************************************************************************/

#ifndef __LIBHANDLER_OPTIONS_H
#define __LIBHANDLER_OPTIONS_H

#include <string>
#include <list>
#include <map>

#include <time.h>
#include <sys/time.h>

#include "noCaseCompare.h"

#include "openframe/OFLock.h"

namespace openaprs {
  using openframe::OFLock;
  using std::string;
  using std::map;
  using std::list;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

#define LIBHANDLER_OPTION_DEFAULT_INT				0
#define LIBHANDLER_OPTION_DEFAULT_STRING			""
#define LIBHANDLER_OPTION_DEFAULT_BOOL				false

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class Option {
  public:

    Option();
    virtual ~Option();

    /*********************
     ** Boolean Members **
     *********************/

    const bool getBoolean() const
      { return myBoolean; }

    void setBoolean(const bool setMe)
      { myBoolean = setMe; }

    /*********************
     ** Integer Members **
     *********************/

    const int getInteger() const
      { return myInteger; }

    void setInteger(const int setMe)
      { myInteger = setMe; }

    /********************
     ** String Members **
     ********************/

    const string &getString() const
      { return myString; }

    void setString(const string &setMe)
      { myString = setMe; }

  protected:
    bool myBoolean;			// boolean value
    int myInteger;			// my integer
    string myString;			// my string
};

class OptionsList {
  public:
    OptionsList();
    virtual ~OptionsList();

    typedef map<string, Option, noCaseCompare> optionMapType;

    /********************
     ** Option Members **
     ********************/

    const bool addOption(const string &, const int, const bool, const string &, const bool);
    const bool addOption(const string &, const int, const bool, const string &);
    const bool addOption(const string &, const string &);
    const bool addOption(const string &, const char *);
    const bool addOption(const string &, const int);
    const bool addOption(const string &, const long int);
    const bool addOption(const string &, const unsigned int);
    const bool incOption(const string &, const int);
    const bool toggleOption(const string);
    const bool getOptionBool(const string &);
    const bool defaultOptionBool(const string &, const bool);
    const bool removeOption(const string &);
    const bool isOption(const string &);
    const int clearOptions();
    const int getOptionInt(const string &);
    const int defaultOptionInt(const string &, const int);
    const string getOptionString(const string &);
    const string defaultOptionString(const string &, const string &);
    const unsigned int matchOptions(const string &, list<string> &);
    const unsigned int removeOptions(const string &);
    void showOptionTree(const string, unsigned int,
                        const unsigned int, string, list<string> &);

    const unsigned int optionSize() const
    { return optionList.size(); }

    // iterators!
    inline optionMapType::iterator optionBegin()
      { return optionList.begin(); }

    inline optionMapType::iterator optionEnd()
      { return optionList.end(); }

  private:
    void _showOptionTree(const string, unsigned int,
                        const unsigned int, string, list<string> &);

    optionMapType optionList;		// option list
    OFLock mutex;

  protected:
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

}
#endif
