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

#include <openframe/config.h>

#include <new>
#include <string>
#include <sstream>
#include <cassert>
#include <iomanip>

#include <openframe/Result.h>
#include <openframe/StringTool.h>
#include <openframe/StringToken.h>

namespace openframe {

  Result::Result(const std::string &fields) {
    StringToken st;

    if (!fields.size())
      throw Result_Exception("fields may not be empty string");

    st.setDelimiter(',');
    st = fields;

    if (!st.size())
      throw Result_Exception("no fields found in comma separated list");

    for(int i=0; i < st.size(); i++)
      _fields.push_back( Field(st[i]) );
  } // Result::Result

  Result::~Result() {
  } // Result::~Result

  void Result::add(Row &row) {
    if (row.size() != num_fields())
      throw Result_Exception("row fields does not match declared expected number of fields");

    push_back(row);
  } // Result::add

  const Result::rowListType::size_type Result::num_rows() const {
    return size();
  } // Result::num_rows

  const Result::fieldListType::size_type Result::num_fields() const {
    return _fields.size();
  } // Result::num_rows

  const std::string &Result::field_name(const int i) {
    return _fields[i].name();
  } // Result::field_name

  void Result::print2(Result &res, const double seconds = -1.0) {
    map<size_t, size_t> maxFieldLen;
    rowListSizeType j;
    size_t maxRowLen = 0;
    size_t fieldLen = 0;
    size_t i;
    std::string sep = "";
    std::string fieldSep = "";
    std::string field;
    std::string value;

    if (!res.num_rows()) {
      std::cout << res.num_rows() << " row" << (res.num_rows() == 1 ? "" : "s") << " in set (" << std::fixed << std::setprecision(5) << seconds << " seconds)" << std::endl << std::endl;
      return;
    } // if

    for(i=0; i < res.num_fields(); i++) {
      // populate map with head size
      fieldLen = res.field_name(i).length();
      maxFieldLen[i] = fieldLen;
    } // for

    for(j=0; j < res.num_rows(); j++) {
      for(i=0; i < res.num_fields(); i++) {
        value = safe(res[j][i]);
        maxFieldLen[i] = RESULT_MAX(maxFieldLen[i], value.length());
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

    std::cout << sep << std::endl;
    std::cout << "| ";
    for(i=0; i < res.num_fields(); i++) {
      field = res.field_name(i);
      StringTool::pad(field, " ", maxFieldLen[i]);
      if (i)
        std::cout << " | ";
      std::cout << field;
    } // for
    std::cout << " |" << std::endl;
    std::cout << sep << std::endl;

    // now one more loop to print out stuffs
    for(j=0; j < res.num_rows(); j++) {
      std::cout << "| ";
      for(i=0; i < res.num_fields(); i++) {
        value = safe(res[j][i]);
        StringTool::pad(value, " ", maxFieldLen[i]);
        if (i)
          std::cout << " | ";
        std::cout << value;
      } // for
      std::cout << " |" << std::endl;
    } // for
    std::cout << sep << std::endl;
    std::cout << res.num_rows() << " row" << (res.num_rows() == 1 ? "" : "s") << " in set (" << std::fixed << std::setprecision(5) << seconds << " seconds)" << std::endl << std::endl;
  } // Result::print2

  const std::string Result::safe(const std::string &buf) {
    std::stringstream s;

    s.str("");
    for(size_t i=0; i < buf.length(); i++) {
      if (((int) buf[i] < 32 || (int) buf[i] > 126) && (int) buf[i] != 10)
        s << "\\x" << StringTool::char2hex(buf[i]);
      else
        s << buf[i];
    } // for

    return s.str();
  } // Result::safe

  const std::string Result::wordwrap(const std::string &buf, const size_t wrap) {
    std::stringstream s;
    size_t c = 0;

    if (wrap == 0)
      return buf;

    s.str("");
    c=1;
    for(size_t i=0; i < buf.length(); i++) {
      if (buf[i] == '\n')
        c = 0;

      if (c == wrap) {
        s << " $\n -> ";
        c = 5;
      } // if

      s << buf[i];
      c++;
    } // for

    return s.str();
  } // Result::wordwrap

  void Result::print(Result &res) {
    Result::print(res, 0, -1.0);
  } // Result::print

  void Result::print(Result &res, const size_t wrap) {
    Result::print(res, wrap, -1.0);
  } // Result::print

  void Result::print(Result &res, const size_t wrap, const double seconds = 0.0) {
    map<size_t, size_t> maxFieldLen;
    rowListSizeType j;
    size_t maxRowLen = 0;
    size_t fieldLen = 0;
    size_t i;
    std::string sep = "";
    std::string fieldSep = "";
    std::string field;
    std::string value;

    if (!res.num_rows()) {
      std::cout << res.num_rows() << " row" << (res.num_rows() == 1 ? "" : "s") << " in set";
      if (seconds != -1.0)
        std::cout << " (" << std::fixed << std::setprecision(5) << seconds << " seconds)" << std::endl << std::endl;
      else
        std::cout << std::endl << std::endl;
      return;
    } // if

    for(i=0; i < res.num_fields(); i++) {
      // populate map with head size
      fieldLen = res.field_name(i).length();
      maxFieldLen[i] = fieldLen;
    } // for

    for(j=0; j < res.num_rows(); j++) {
      for(i=0; i < res.num_fields(); i++) {
        value = Result::wordwrap(safe(res[j][i]), wrap);
        maxFieldLen[i] = RESULT_MAX(maxFieldLen[i], Result::maxNewlineLen(value) );
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

    std::cout << sep << std::endl;
    std::cout << "| ";
    for(i=0; i < res.num_fields(); i++) {
      field = res.field_name(i);
      if (i) std::cout << (field == " " ? "   " : " | ");
      StringTool::pad(field, " ", maxFieldLen[i]);
      std::cout << field;
    } // for
    std::cout << " |" << std::endl;
    std::cout << sep << std::endl;

    // now one more loop to print out stuffs
    for(j=0; j < res.num_rows(); j++) {
      // loop for newlines and try not to screw up the formatting
      size_t newlines = 0;
      for(size_t n=0; n == 0 || newlines; n++) {
        std::stringstream s;
        newlines = 0;
        s << "| ";
        for(i=0; i < res.num_fields(); i++) {
          if (lineForNewlineRow(wordwrap(safe(res[j][i]), wrap), value, n))
            newlines++;

          StringTool::pad(value, " ", maxFieldLen[i]);
          if (i)
            s << " | ";
          s << value;
        } // for
        s << " |" << std::endl;

        if (newlines)
          std::cout << s.str();
      } // for

    } // for
    std::cout << sep << std::endl;

    std::cout << res.num_rows() << " row" << (res.num_rows() == 1 ? "" : "s") << " in set";
    if (seconds != -1.0)
      std::cout << " (" << std::fixed << std::setprecision(5) << seconds << " seconds)" << std::endl << std::endl;
    else
      std::cout << std::endl << std::endl;
  } // Result::print

  void Result::print(const std::string &label, Result &res) {
    std::string out;
    Result::print(label, res, out);
    std::cout << out.str();
  } // Result::print

  void Result::print(const std::string &label, Result &res, std::string &ret) {
    std::string sep = "";
    std::string fieldSep = "";
    std::string name;
    std::string value;
    std::string l = "";
    size_t maxFieldLen = 0;
    size_t maxValueLen = 0;
    size_t i;
    Result::size_type j;
    std::stringstream out;

    for(i=0; i < res.num_fields(); i++)
      maxFieldLen = RESULT_MAX(res.field_name(i).length(), maxFieldLen);

    for(j=0; j < res.num_rows(); j++) {
      for(i=0; i < res.num_fields(); i++) {
        value = string( res[j][i] );
        value = wordwrap(safe(value), 80);
        maxValueLen = RESULT_MAX(Result::maxNewlineLen(value), maxValueLen);
      } // for
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
    out << sep << std::endl;
    out << "| " << l << " |" << std::endl;
    out << fieldSep << std::endl;
    for(j=0; j < res.num_rows(); j++) {
      for(i=0; i < res.num_fields(); i++) {
        size_t newlines=0;
        for(size_t n=0; n == 0 || newlines; n++) {
          newlines=0;
          name = (n == 0 ? res.field_name(i) : "");
          if (Result::lineForNewlineRow(wordwrap(safe(res[j][i]), 80), value, n))
            newlines++;
          StringTool::pad(name, " ", maxFieldLen);
          StringTool::pad(value, " ", maxValueLen);
          if (newlines)
            out << "| " << name << "| " << value << "|" << std::endl;
        } // for
      } // for
      out << fieldSep << std::endl;
    } // for
//    out << fieldSep << std::endl;
    out << res.num_rows() << " row" << (res.num_rows() == 1 ? "" : "s") << " in set" << std::endl << std::endl;

    ret = out.str();
  } // Result::print

  const size_t Result::maxNewlineLen(const std::string &buf) {
    StringToken st;
    size_t len = 0;

    st.setDelimiter('\n');
    st = buf;

    if (!st.size())
      return 0;

    for(size_t i=0; i < st.size(); i++)
      len = RESULT_MAX(len, st[i].length());

    return len;
  } // Result::maxNewlineLen

  const bool Result::lineForNewlineRow(const std::string &buf, std::string &ret, const size_t i) {
    StringToken st;

    st.setDelimiter('\n');
    st = buf;

    if (i >= st.size()) {
      ret = "";
      return false;
    } // if

    ret = st[i];

    return true;
  } // Result::lineForNewlineRow

  const bool Result::diff(Result &res1, Result &res2) {
    return Result::diff(res1, res2, "", false);
  } // Result::diff

  const bool Result::diff(Result &res1, Result &res2, const std::string &exclude, const bool silent) {
    Result::size_type num_rows1 = res1.num_rows();
    Result::size_type num_rows2 = res2.num_rows();
    Result::size_type num_fields1 = res1.num_fields();
    Result::size_type num_fields2 = res2.num_fields();
    size_t num_diff = 0;

    // should have used diffset?
    if (num_rows1 > 1 || num_rows2 > 1)
      return diffset(res1, res2, exclude, silent);

    if (num_rows1 != num_rows2) {
      if (!silent)
        std::cout << "Rows counts do not match; res1 " << num_rows1 << " != res2 " << num_rows2 << std::endl;
      return true;
    } // if

    if (num_fields1 != num_fields2) {
      if (!silent)
        std::cout << "Number of fields do not match; res1 " << num_fields1 << " != res2 " << num_fields2 << std::endl;
      return true;
    } // if

    Result r("Row,Field Name,Result 1,Result 2");
    for(Result::size_type i=0; i < num_rows1; i++) {
      for(Result::size_type j=0; j < num_fields1; j++) {
        StringTool::regexMatchListType rl;

        // skip any fields from our exclusion regex
        if (exclude.length()
            && StringTool::ereg(exclude, res1.field_name(j), rl))
          continue;

        bool isMatch = (res1[i][j] == res2[i][j]);
        if (isMatch)
          continue;

        // short cirtuit if silent was requested
        // saves time and cycles
        if (silent)
          return true;

        std::stringstream out;
        out << i+1;
        Serialize s;
        s.add( out.str() );
        s.add( string(res1.field_name(j)) );
        s.add( string(res1[i][j]) );
        s.add( string(res2[i][j]) );
        Row row(s.compile());
        r.add(row);
        num_diff++;
      } // for
    } // for

    if (!r.size()) {
      if (!silent)
        std::cout << "Results match." << std::endl;
      return false;
    } // if

    if (!silent)
      Result::print(r);

    return true;
  } // Result::diff

  const bool Result::diffset(Result &res1, Result &res2, const std::string &exclude, const bool silent) {
    Result::size_type num_fields1 = res1.num_fields();
    Result::size_type num_fields2 = res2.num_fields();

    if (num_fields1 != num_fields2) {
      if (!silent)
        std::cout << "Number of fields do not match; res1 " << num_fields1 << " != res2 " << num_fields2 << std::endl;
      return true;
    } // if

    // match our field names up
    for(Result::size_type i=0; i < num_fields1; i++) {
      bool isMatch = (res1.field_name(i) == res2.field_name(i));
      if (!isMatch) {
        if (!silent)
          std::cout << "Field names don't match, must be identical query; res1 " << res1.field_name(i) << " != res2 " << res2.field_name(i) << std::endl;
        return true;
      } // if
    } // for

    Result result("Row,Field Name,Result 1,Result 2");

    Result::diffsetone(res1, res2, exclude, true, result);
    Result::diffsetone(res2, res1, exclude, false, result);

    if (!result.size()) {
      if (!silent)
        std::cout << "Results match." << std::endl;
      return false;
    } // if

    if (!silent)
      Result::print(result);

    return true;
  } // Result::diffset

  void Result::diffsetone(Result &res1, Result &res2, const std::string &exclude, const bool first, Result &result) {
    Result::size_type num_rows1 = res1.num_rows();
    Result::size_type num_rows2 = res2.num_rows();
    Result::size_type num_fields1 = res1.num_fields();
    Result::size_type num_fields2 = res2.num_fields();

    // now try and pair up each result
    for(Result::size_type i=0; i < num_rows1; i++) {
      std::stringstream out;
      std::stringstream fields;
      std::stringstream values;
      size_t num_mismatch = 0;
      out << (!first ? "*" : "" ) << i+1;
      for(Result::size_type j=0; j < num_fields1; j++) {
        std::string value1 = string( res1[i][j] );
        bool isMatch = false;
        StringTool::regexMatchListType rl;

        // skip any fields from our exclusion regex
        if (exclude.length()
            && StringTool::ereg(exclude, res1.field_name(j), rl))
          continue;

        fields << (fields.str().length() ? "," : "") << res1.field_name(j);
        values << (values.str().length() ? "," : "") << string(res1[i][j]);

        for(Result::size_type k=0; k < num_rows2; k++) {
          std::string value2 = string(res2[k][j]);
          isMatch = (value1 == value2);

          if (isMatch) {
            break;
          } // if
        } // for

        // found no matching fields in other result
        if (!isMatch)
          num_mismatch++;
      } // for

      if (num_mismatch) {
          Serialize s;
          s.add( out.str() );
          s.add( fields.str() );
          if (first) {
            s.add( values.str() );
            s.add( string(""));
          } // if
          else {
            s.add( string(""));
            s.add( values.str() );
          } // else
          Row row(s.compile());
          result.add(row);
      } // if
    } // for
  } // Result::diffsetone

/*
  void Result::print(const std::string &label, Result &res) {
    std::string sep = "";
    std::string fieldSep = "";
    std::string name;
    std::string value;
    std::string l = "";
    size_t maxFieldLen = 0;
    size_t maxValueLen = 0;
    size_t i;
    Result::size_type j;

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
    std::cout << sep << std::endl;
    std::cout << "| " << l << " |" << std::endl;
    std::cout << fieldSep << std::endl;
    for(j=0; j < res.num_rows(); j++) {
      for(i=0; i < res.num_fields(); i++) {
        name = res.field_name(i);
        value = res[j][res.field_name(i).c_str()].c_str();
        StringTool::pad(name, " ", maxFieldLen);
        StringTool::pad(value, " ", maxValueLen);
        std::cout << "| " << name << "| " << value << "|" << std::endl;
      } // for
    } // for
    std::cout << fieldSep << std::endl;

  } // Result::print

  void Result::makeListFromField(const std::string &name, Result &res, std::string &list) {
    Result::size_type j;

    list = "";

    for(j=0; j < res.num_rows(); j++) {
      if (list.length())
        list += ", ";
      list += res[j][name.c_str()].c_str();
    } // for
  } // Result::makeListFromField
*/

} // namespace openframe
