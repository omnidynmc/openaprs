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
 $Id: LineBuffer.h,v 1.1 2005/11/21 18:16:03 omni Exp $
 **************************************************************************/

#ifndef __LINEBUFFER_H
#define __LINEBUFFER_H

#include <string>
#include <map>
#include <list>

namespace openaprs {

using std::string;
using std::map;
using std::list;
using std::pair;

/*
 **************************************************************************
 ** General Defines                                                      **
 **************************************************************************
 */

/*
 **************************************************************************
 ** Structures                                                           **
 **************************************************************************
 */

class LineBuffer {
  public:
    LineBuffer(char = '\n');		// constructor
    virtual ~LineBuffer();		// destructor

    bool readLine(string &);		// read a line from the buffer
    bool remove(int);			// remove all user modes in channel

    void add(const string &);		// add to buffer
    void flush();			// flush buffer

    virtual string::size_type size() const { return buf.size(); }
    virtual string::size_type length() const { return buf.length(); }
    virtual string::size_type capacity() const { return buf.capacity(); }

    const int dumpBuffer(string &);

  private:
    inline LineBuffer & operator+=(const char *);
    inline LineBuffer & operator+=(const string &);
    inline LineBuffer & operator+=(const LineBuffer &);
    inline LineBuffer & operator=(const string &);
    inline LineBuffer & operator=(const char *);
    inline LineBuffer & operator=(const LineBuffer &);
//    inline const string & operator() const { return buf; };

  protected:
    string buf;
    char delim;
};

/*
 **************************************************************************
 ** Macro's                                                              **
 **************************************************************************
 */

/*
 **************************************************************************
 ** Proto types                                                          **
 **************************************************************************
 */

}
#endif
