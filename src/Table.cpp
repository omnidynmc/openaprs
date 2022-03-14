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

#include <new>
#include <string>
#include <sstream>
#include <cassert>
#include <iomanip>

#include "Table.h"
#include "StringTool.h"
#include "StringToken.h"

#include "config.h"

namespace openaprs {
  using namespace std;
  using aprs::StringTool;

  Table::Table(const string &fields) {
    StringToken st;

    if (!fields.size())
      throw Table_Exception("fields may not be empty string");

    st.setDelimiter(',');
    st = fields;

    if (!st.size())
      throw Table_Exception("no fields found in comma separated list");

    for(int i=0; i < st.size(); i++)
      _fields.push_back( Field(st[i]) );
  } // Table::Table

  Table::~Table() {
  } // Table::~Table

  void Table::add(Row &row) {
    if (row.size() != num_fields())
      throw Table_Exception("row fields does not match declared expected number of fields");

    push_back(row);
  } // Table::add

  const Table::rowListType::size_type Table::num_rows() const {
    return size();
  } // Table::num_rows

  const Table::fieldListType::size_type Table::num_fields() const {
    return _fields.size();
  } // Table::num_rows

  const string &Table::field_name(const int i) {
    return _fields[i].name();
  } // Table::field_name

  void Table::print2(Table &res, const double seconds = 0.0) {
    map<size_t, size_t> maxFieldLen;
    rowListSizeType j;
    size_t maxRowLen = 0;
    size_t fieldLen = 0;
    size_t i;
    string sep = "";
    string fieldSep = "";
    string field;
    string value;

    if (!res.num_rows()) {
      cout << res.num_rows() << " row" << (res.num_rows() == 1 ? "" : "s") << " in set (" << fixed << setprecision(5) << seconds << " seconds)" << endl << endl;
      return;
    } // if

    for(i=0; i < res.num_fields(); i++) {
      // populate map with head size
      fieldLen = res.field_name(i).length();
      maxFieldLen[i] = fieldLen;
    } // for

    for(j=0; j < res.num_rows(); j++) {
      for(i=0; i < res.num_fields(); i++) {
        value = res[j][i];
        maxFieldLen[i] = TABLE_MAX(maxFieldLen[i], value.length());
      } // for
    } // for

    // create the sep
    for(i=0; i < maxFieldLen.size(); i++) {
      fieldSep = "+";
      StringTool::pad(fieldSep, "-", maxFieldLen[i]+3);
      sep += fieldSep;
      maxRowLen += maxFieldLen[i];
    } // for
    sep += "+";

    maxRowLen += (maxFieldLen.size()-1) * 3;

    cout << sep << endl;
    cout << "| ";
    for(i=0; i < res.num_fields(); i++) {
      field = res.field_name(i);
      StringTool::pad(field, " ", maxFieldLen[i]);
      if (i)
        cout << " | ";
      cout << field;
    } // for
    cout << " |" << endl;
    cout << sep << endl;

    // now one more loop to print out stuffs
    for(j=0; j < res.num_rows(); j++) {
      cout << "| ";
      for(i=0; i < res.num_fields(); i++) {
        value = res[j][i].c_str();
        StringTool::pad(value, " ", maxFieldLen[i]);
        if (i)
          cout << " | ";
        cout << value;
      } // for
      cout << " |" << endl;
    } // for
    cout << sep << endl;
    cout << res.num_rows() << " row" << (res.num_rows() == 1 ? "" : "s") << " in set (" << fixed << setprecision(5) << seconds << " seconds)" << endl << endl;
  } // Table::print2

  void Table::print(Table &res, const double seconds = 0.0) {
    map<size_t, size_t> maxFieldLen;
    rowListSizeType j;
    size_t maxRowLen = 0;
    size_t fieldLen = 0;
    size_t i;
    string sep = "";
    string fieldSep = "";
    string field;
    string value;

    if (!res.num_rows()) {
      cout << res.num_rows() << " row" << (res.num_rows() == 1 ? "" : "s") << " in set (" << fixed << setprecision(5) << seconds << " seconds)" << endl << endl;
      return;
    } // if

    for(i=0; i < res.num_fields(); i++) {
      // populate map with head size
      fieldLen = res.field_name(i).length();
      maxFieldLen[i] = fieldLen;
    } // for

    for(j=0; j < res.num_rows(); j++) {
      for(i=0; i < res.num_fields(); i++) {
        value = res[j][i];
        maxFieldLen[i] = TABLE_MAX(maxFieldLen[i], Table::maxNewlineLen(value) );
      } // for
    } // for

    // create the sep
    for(i=0; i < maxFieldLen.size(); i++) {
      fieldSep = "+";
      StringTool::pad(fieldSep, "-", maxFieldLen[i]+3);
      sep += fieldSep;
      maxRowLen += maxFieldLen[i];
    } // for
    sep += "+";

    maxRowLen += (maxFieldLen.size()-1) * 3;

    cout << sep << endl;
    cout << "| ";
    for(i=0; i < res.num_fields(); i++) {
      field = res.field_name(i);
      StringTool::pad(field, " ", maxFieldLen[i]);
      if (i)
        cout << " | ";
      cout << field;
    } // for
    cout << " |" << endl;
    cout << sep << endl;

    // now one more loop to print out stuffs
    for(j=0; j < res.num_rows(); j++) {
      // loop for newlines and try not to screw up the formatting
      size_t newlines = 0;
      for(size_t n=0; n == 0 || newlines; n++) {
        stringstream s;
        newlines = 0;
        s << "| ";
        for(i=0; i < res.num_fields(); i++) {
          if (lineForNewlineRow(res[j][i].c_str(), value, n))
            newlines++;

          StringTool::pad(value, " ", maxFieldLen[i]);
          if (i)
            s << " | ";
          s << value;
        } // for
        s << " |" << endl;

        if (newlines)
          cout << s.str();
      } // for

    } // for
    cout << sep << endl;
    cout << res.num_rows() << " row" << (res.num_rows() == 1 ? "" : "s") << " in set (" << fixed << setprecision(5) << seconds << " seconds)" << endl << endl;
  } // Table::print

  const size_t Table::maxNewlineLen(const string &buf) {
    StringToken st;
    size_t len = 0;

    st.setDelimiter('\n');
    st = buf;

    if (!st.size())
      return 0;

    for(size_t i=0; i < st.size(); i++)
      len = TABLE_MAX(len, st[i].length());

    return len;
  } // Table::maxNewlineLen

  const bool Table::lineForNewlineRow(const string &buf, string &ret, const size_t i) {
    StringToken st;

    st.setDelimiter('\n');
    st = buf;

    if (i >= st.size()) {
      ret = "";
      return false;
    } // if

    ret = st[i];

    return true;
  } // Table::lineForNewlineRow

/*
  void Table::print(const string &label, resultType &res) {
    string sep = "";
    string fieldSep = "";
    string name;
    string value;
    string l = "";
    size_t maxFieldLen = 0;
    size_t maxValueLen = 0;
    size_t i;
    resultSizeType j;

    for(i=0; i < res.num_fields(); i++)
      maxFieldLen = DBI_MAX(res.field_name(i).length(), maxFieldLen);

    for(j=0; j < res.num_rows(); j++) {
      for(i=0; i < res.num_fields(); i++)
        maxValueLen = DBI_MAX(res[j][res.field_name(i).c_str()].length(), maxValueLen);
    } // for

    maxFieldLen += 1;
    maxValueLen += 1;

    sep = "+";
    StringTool::pad(sep, "-", maxFieldLen+maxValueLen+4);
    sep += "+";

    fieldSep = "+";
    StringTool::pad(fieldSep, "-", maxFieldLen+2);
    fieldSep += "+";
    StringTool::pad(fieldSep, "-", maxFieldLen+maxValueLen+4);
    fieldSep += "+";

    l = label;
    StringTool::pad(l, " ", maxFieldLen+maxValueLen+1);

    // loop through result
    cout << sep << endl;
    cout << "| " << l << " |" << endl;
    cout << fieldSep << endl;
    for(j=0; j < res.num_rows(); j++) {
      for(i=0; i < res.num_fields(); i++) {
        name = res.field_name(i);
        value = res[j][res.field_name(i).c_str()].c_str();
        StringTool::pad(name, " ", maxFieldLen);
        StringTool::pad(value, " ", maxValueLen);
        cout << "| " << name << "| " << value << "|" << endl;
      } // for
    } // for
    cout << fieldSep << endl;

  } // Table::print

  void Table::makeListFromField(const string &name, resultType &res, string &list) {
    resultSizeType j;

    list = "";

    for(j=0; j < res.num_rows(); j++) {
      if (list.length())
        list += ", ";
      list += res[j][name.c_str()].c_str();
    } // for
  } // Table::makeListFromField
*/

} // namespace openaprs
