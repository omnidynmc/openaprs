/**************************************************************************
 ** OpenFrame Library                                                    **
 **************************************************************************
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

#ifndef __LIBOPENFRAME_RESULT_H
#define __LIBOPENFRAME_RESULT_H

#include <string>
#include <vector>
#include <cassert>

#include "openframe/OpenFrame_Abstract.h"
#include "openframe/StringToken.h"
#include "openframe/StringTool.h"
#include "openframe/Serialize.h"

namespace openframe {
  using std::string;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

#define RESULT_MAX(a, b)           (a > b ? a : b)

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

  class Result_Exception : public OpenFrame_Exception {
    public:
      Result_Exception(const string message) throw() : OpenFrame_Exception(message) {
      } // Result_Exception

    private:
  }; // class Result_Exception

  class Result : public std::vector<Row> {
    public:
      Result(const string &);
      Result() { }
      virtual ~Result();

      // ### Type Definitions  ###
      typedef std::vector<Field> fieldListType;
      typedef std::vector<Row> rowListType;
      typedef rowListType::size_type rowListSizeType;

      const rowListType::size_type num_rows() const;
      const fieldListType::size_type num_fields() const;
      const string &field_name(const int i);
      void add(Row &);
      static void print(Result &);
      static void print(Result &, const size_t);
      static void print(Result &, const size_t, const double);
      static void print2(Result &, const double);
      static void print(const string &, Result &);
      static void print(const string &, Result &, string &);
      static const size_t maxNewlineLen(const string &);
      static const bool lineForNewlineRow(const string &, string &, const size_t);
      static const string safe(const string &);
      static const string wordwrap(const string &, const size_t);
      static const bool diff(Result &, Result &);
      static const bool diff(Result &, Result &, const string &, const bool);
      static const bool diffset(Result &, Result &, const string &, const bool);

    protected:
      static void diffsetone(Result &, Result &, const string &, const bool, Result &);

    private:
      fieldListType _fields;
  }; // class Result


/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // namespace openframe

#endif
