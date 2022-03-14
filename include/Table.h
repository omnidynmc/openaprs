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
 $Id: Server.cpp,v 1.9 2006/03/21 20:55:12 omni Exp $
 **************************************************************************/

#ifndef OPENAPRS_TABLE_H
#define OPENAPRS_TABLE_H

#include <string>
#include <vector>
#include <cassert>

#include "App_Log.h"
#include "OpenAPRS_Abstract.h"
#include "StringTool.h"
#include "Serialize.h"

#include "openframe/StringToken.h"

namespace openaprs {
  using openframe::StringToken;
  using std::string;
  using aprs::StringTool;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

#define TABLE_MAX(a, b)           (a > b ? a : b)

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/
  class Field {
    public:
      Field(const string &name) : _name(name) { }
      ~Field() { }

      const string &name() const { return _name; }

    protected:
    private:
      string _name;
  }; // class Field

  class Row : public std::vector<string> {
    public:
      Row(const string &value) {
        assert(value.length());
        Serialize v(value);
        assert(v.size());

        for(size_t i=0; i < v.size(); i++) {
          string buf = v[i];
          StringTool::replace("\n", "\\n", buf);
          push_back(v[i]);
        } // for
      } // Row
      ~Row() { }

    protected:
    private:
  }; // class Row

  class Table_Exception : public OpenAPRSAbstract_Exception {
    public:
      Table_Exception(const string message) throw() : OpenAPRSAbstract_Exception(message) {
      } // Table_Exception

    private:
  }; // class Table_Exception

  class Table : public std::vector<Row> {
    public:
      Table(const string &);
      virtual ~Table();

      // ### Type Definitions  ###
      typedef std::vector<Field> fieldListType;
      typedef std::vector<Row> rowListType;
      typedef rowListType::size_type rowListSizeType;

      const rowListType::size_type num_rows() const;
      const fieldListType::size_type num_fields() const;
      const string &field_name(const int i);
      void add(Row &);
      static void print(Table &, const double);
      static void print2(Table &, const double);
      static const size_t maxNewlineLen(const string &);
      static const bool lineForNewlineRow(const string &, string &, const size_t);

    protected:
    private:
      fieldListType _fields;
  }; // class Table


/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // openaprs

#endif
