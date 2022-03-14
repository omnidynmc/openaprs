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

#ifdef FEAT_MYSQL

#include <new>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include <mysql++.h>

#include <openframe/DBI.h>
#include <openframe/StringTool.h>
#include <openframe/StringToken.h>
#include <openframe/Result.h>
#include <openframe/Serialize.h>
#include <openframe/Vars.h>

namespace openframe {
  const size_t DBI::DEFAULT_WRAP	=	50;

  DBI::DBI(const std::string &db, const std::string &host, const std::string &user, const std::string &pass) :
           _db(db), _host(host), _user(user), _pass(pass) {
    try {
      _sqlpp = new mysqlpp::Connection(true);
      _sqlpp->set_option(new mysqlpp::ReconnectOption(true));
      _sqlpp->set_option(new mysqlpp::MultiResultsOption(true));
      _sqlpp->set_option(new mysqlpp::MultiStatementsOption(true));
      _sqlpp->set_option(new mysqlpp::SetCharsetNameOption("utf8"));
    } // try
    catch(std::bad_alloc xa) {
      assert(false);
    } // catch

    _connect();

  } // DBI::DBI

  DBI::~DBI() {
    if (_sqlpp->connected())
      _sqlpp->disconnect();

    delete _sqlpp;
  } // DBI:~DBI

  const bool DBI::_connect() {
    try {
      _sqlpp->connect(_db.c_str(), _host.c_str(), _user.c_str(), _pass.c_str(), 3306);
    } // try
    catch(mysqlpp::ConnectionFailed e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
      return false;
    } // catch

    return true;
  } // DBI::_connect

  const DBI::resultSizeType DBI::query(const std::string &sqls, resultType &res) {
    mysqlpp::Query query = _sqlpp->query(sqls);

    try {
      res = query.store();
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI::query

  void DBI::print(resultType &res) {
    DBI::print(res, DBI::DEFAULT_WRAP);
  } // DBI::print

  void DBI::print(resultType &res, const size_t wrap) {
    std::string out;
    DBI::print(res, wrap, out);
    std::cout << out;
  } // DBI::print

  void DBI::print(resultType &res, const size_t wrap, std::string &ret) {
    map<size_t, size_t> maxFieldLen;
    resultSizeType j;
    size_t maxRowLen = 0;
    size_t fieldLen = 0;
    size_t i;
    std::string sep = "";
    std::string fieldSep = "";
    std::string field;
    std::string value;
    std::stringstream out;

    for(i=0; i < res.num_fields(); i++) {
      // populate map with head size
      fieldLen = res.field_name(i).length();
      maxFieldLen[i] = fieldLen;
    } // for

    for(j=0; j < res.num_rows(); j++) {
      for(i=0; i < res.num_fields(); i++) {
        res[j][i].to_string(value);
        value = DBI::wordwrap(safe(value), wrap);
        maxFieldLen[i] = DBI_MAX(maxFieldLen[i], DBI::maxNewlineLen(value) );
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

    out << sep << std::endl;
    out << "| ";
    for(i=0; i < res.num_fields(); i++) {
      field = res.field_name(i);
      StringTool::pad(field, " ", maxFieldLen[i]);
      if (i)
        out << " | ";
      out << field;
    } // for
    out << " |" << std::endl;
    out << sep << std::endl;

    // now one more loop to print out stuffs
    for(j=0; j < res.num_rows(); j++) {
      size_t newlines=0;
      for(size_t n=0; n == 0 || newlines; n++) {
        std::stringstream s;
        newlines=0;
        s << "| ";
        for(i=0; i < res.num_fields(); i++) {
          std::string cmp_value;
          res[j][i].to_string(cmp_value);
          if (lineForNewlineRow(wordwrap(safe(cmp_value), wrap), value, n))
            newlines++;

          StringTool::pad(value, " ", maxFieldLen[i]);
          if (i)
            s << " | ";
          s << value;
        } // for
        s << " |" << std::endl;

        if (newlines)
          out << s.str();
      } // for
    } // for
    out << sep << std::endl;
    out << res.num_rows() << " row" << (res.num_rows() == 1 ? "" : "s") << " in set" << std::endl << std::endl;

    ret = out.str();
  } // DBI::print

  const std::string DBI::wordwrap(const std::string &buf, const size_t wrap) {
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
  } // DBI::wordwrap

  const size_t DBI::maxNewlineLen(const std::string &buf) {
    StringToken st;
    size_t len = 0;

    st.setDelimiter('\n');
    st = buf;

    if (!st.size())
      return 0;

    for(size_t i=0; i < st.size(); i++)
      len = DBI_MAX(len, st[i].length());

    return len;
  } // DBI::maxNewlineLen

  const bool DBI::lineForNewlineRow(const std::string &buf, std::string &ret, const size_t i) {
    StringToken st;

    st.setDelimiter('\n');
    st = buf;

    if (i >= st.size()) {
      ret = "";
      return false;
    } // if

    ret = st[i];

    return true;
  } // DBI::lineForNewlineRow

  void DBI::print(const std::string &label, resultType &res) {
    std::string out;
    DBI::print(label, res, out);
    std::cout << out;
  } // DBI::print

  void DBI::print(const std::string &label, resultType &res, std::string &ret) {
    std::string sep = "";
    std::string fieldSep = "";
    std::string name;
    std::string value;
    std::string l = "";
    size_t maxFieldLen = 0;
    size_t maxValueLen = 0;
    size_t i;
    resultSizeType j;
    std::stringstream out;

    for(i=0; i < res.num_fields(); i++)
      maxFieldLen = DBI_MAX(res.field_name(i).length(), maxFieldLen);

    for(j=0; j < res.num_rows(); j++) {
      for(i=0; i < res.num_fields(); i++) {
        res[j][i].to_string(value);
        value = DBI::wordwrap(safe(value), DEFAULT_WRAP);
        maxValueLen = DBI_MAX(DBI::maxNewlineLen(value), maxValueLen);
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
          std::string cmp_value;
          res[j][i].to_string(cmp_value);
          if (DBI::lineForNewlineRow(DBI::wordwrap(safe(cmp_value), DBI::DEFAULT_WRAP), value, n))
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
  } // DBI::print

  void DBI::combine(resultType &res1, resultType &res2, Result &ret) {
    resultSizeType maxfields = DBI_MAX(res1.num_fields(), res2.num_fields());
    resultSizeType max1 = res1.num_fields();
    resultSizeType max2 = res2.num_fields();

    for(size_t i=0; i < maxfields; i++) {
      Serialize s;
      if (i < max1) {
        s.add(res1.field_name(i));
        std::string value;
        res1[0][i].to_string(value);
        s.add(value);
      } // if
      else {
        s.add(string(""));
        s.add(string(""));
      } // else

      if (i < max2) {
        s.add(res2.field_name(i));
        std::string value;
        res2[0][i].to_string(value);
        s.add(value);
      } // if
      else {
        s.add(string(""));
        s.add(string(""));
      } // else

      openframe::Row r(s.compile());
      ret.add(r);
    } // for

  } // DBI::combine

  void DBI::combine(combineResultType &res, Result &ret) {
    // first we need to find the maximum number of fields to use
    resultSizeType maxfields = 0;
    for(combineResultSizeType i=0; i < res.size(); i++)
      maxfields = DBI_MAX(maxfields, res[i].num_fields());

    for(size_t i=0; i < maxfields; i++) {
      Serialize s;

      for(combineResultSizeType j=0; j < res.size(); j++) {
        resultSizeType max = res[j].num_fields();
        if (i < max) {
          s.add(res[j].field_name(i));
          std::string value;
          res[j][0][i].to_string(value);
          s.add(value);
        } // if
        else {
          s.add(string(""));
          s.add(string(""));
        } // else
      } // for

      openframe::Row r(s.compile());
      ret.add(r);
    } // for

  } // DBI::combine

  const std::string DBI::safe(const std::string &buf) {
    std::stringstream s;

    s.str("");
    for(size_t i=0; i < buf.length(); i++) {
      if (((int) buf[i] < 32 || (int) buf[i] > 126) && (int) buf[i] != 10)
        s << "\\x" << StringTool::char2hex(buf[i]);
      else
        s << buf[i];
    } // for

    return s.str();
  } // DBI::safe

  void DBI::makeListFromField(const std::string &name, resultType &res, std::string &list) {
    makeListFromField(name, ", ", res, list);
  } // DBI::makeListFromField

  void DBI::makeListFromField(const std::string &name, const std::string &sep, resultType &res, std::string &list) {
    resultSizeType j;

    list = "";

    for(j=0; j < res.num_rows(); j++) {
      if (list.length())
        list += sep;
      std::string value;
      res[j][name.c_str()].to_string(value);
      list += value;
    } // for
  } // DBI::makeListFromField

  const bool DBI::diff(resultType &res1, resultType &res2) {
    return DBI::diff(res1, res2, "", false);
  } // DBI::diff

  const bool DBI::diff(resultType &res1, resultType &res2, const std::string &exclude, const bool silent) {
    Result r1 = DBI::convert(res1);
    Result r2 = DBI::convert(res2);
    return Result::diff(r1, r2, exclude, silent);
  } // DBI::diff

  const bool DBI::diffset(resultType &res1, resultType &res2, const std::string &exclude, const bool silent) {
    Result r1 = DBI::convert(res1);
    Result r2 = DBI::convert(res2);
    return Result::diffset(r1, r2, exclude, silent);
  } // DBI::diffset

  void DBI::store(const std::string &name, Result &res) {
    _store[name] = res;
  } // DBI::store

  void DBI::store(const std::string &name, resultType &res) {
    Result result = DBI::convert(res);
    DBI::store(name, result);
  } // DBI::store

  const DBI::storeSizeType DBI::store(storeType &ret) {
    ret = _store;
    return ret.size();
  } // DBI::store

  const bool DBI::restore(const std::string &name, Result &res) {
    storeType::iterator ptr;

    ptr = _store.find(name);
    if (ptr == _store.end())
      return false;

    res = _store[name];

    return true;
  } // DBI::restore

  const bool DBI::restore(const std::string &name, resultType &res) {
    Result result = DBI::convert(res);
    return DBI::restore(name, result);
  } // DBI::restore

  const bool DBI::save(const std::string &file, const std::string &name) {
    storeType::iterator ptr;
    bool isSingle = false;

    if (_store.empty())
      return false;

    if (name.length()) {
      ptr = _store.find(name);
      if (ptr == _store.end())
        return false;

      isSingle = true;
    } // if

    std::ofstream myfile;
    myfile.open(file.c_str());

    if (!myfile.is_open())
      return false;

    if (isSingle) {
      myfile << ":" << ptr->first << std::endl;
      Result res = ptr->second;
      for(Result::size_type i=0; i < res.num_rows(); i++) {
        Vars v;
        for(Result::size_type j=0; j < res.num_fields(); j++) {
          std::string value;
          v.add(res.field_name(j), res[i][j]);
        } // for

        myfile << v.compile() << std::endl;
      } // for
    } // if
    else {
      storeType::iterator sptr;
      for(sptr = _store.begin(); sptr != _store.end(); sptr++) {
        myfile << ":" << sptr->first << std::endl;
        Result res = sptr->second;
        for(Result::size_type i=0; i < res.num_rows(); i++) {
          Vars v;
          for(Result::size_type j=0; j < res.num_fields(); j++) {
            std::string value;
            v.add(res.field_name(j), res[i][j]);
          } // for

          myfile << v.compile() << std::endl;
        } // for
      } // for

    } // else

    myfile.close();

    return true;
  } // DBI::save

  const bool DBI::load(const std::string &filename) {
    StringToken st;
    std::string buf = StringTool::getFileContents(filename);
    std::string name = "";
    size_t num_fields = 0;
    vector<Vars> parsed;

    st.setDelimiter('\n');
    st = buf;

    for(size_t i=0; i < st.size(); i++) {
      std::string line = st[i];
      StringTool::stripcrlf(line);

      if (line.length() < 2) {
        std::cout << "WARNING: blank line or nothing to parse on line " << i+1 << std::endl;
        continue;
      } // if

      // skip comments
      if (line[0] == '#')
        continue;

      // this line has the name of the results
      if (line[0] == ':') {
        if (parsed.size()) {
          Result result(parsed[0].fields());
          for(size_t i=0; i < parsed.size(); i++) {
            Vars v = parsed[i];
            Serialize s;
            for(size_t j=0; j < v.size(); j++) {
              s.add(v[j]);
            } // for
            Row row(s.compile());
            result.add(row);
          } // for
//          Result::print(result);
          store(name, result);
        } // if

        name = line.substr(1, line.length());
        num_fields = 0;
        parsed.clear();
        continue;
      } // if
      // no name? whatever this line is, is invalid
      else if (!name.length()) {
        std::cout << "WARNING: no name to parse arguments on line " << i+1 << std::endl;
        continue;
      } // if

      Vars v(line);
      // did we parse out anything?
      if (!v.size()) {
        std::cout << "WARNING: could not parse line " << i+1 << std::endl;
        continue;
      } // if

      // the number of fields for ALL results in a set must match
      if (num_fields && num_fields != v.size()) {
        std::cout << "WARNING: consistancy match fails, number of fields don't match in result set on line " << i+1 << std::endl;
        continue;
      } // if

      if (num_fields == 0)
        num_fields = v.size();

      parsed.push_back(v);
    } // for

    return true;
  } // DBI::load

  const bool DBI::purge(const std::string &name) {
    storeType::iterator ptr;

    ptr = _store.find(name);
    if (ptr == _store.end())
      return false;

    _store.erase(ptr);
    return true;
  } // DBI::purge

  Result DBI::convert(resultType &res, const std::string &exclude) {
    if (!res.size())
      throw DBI_Exception("result has no rows");

    std::stringstream fields;
    for(resultSizeType i=0; i < res.num_fields(); i++) {
      StringTool::regexMatchListType rl;

      // skip any fields from our exclusion regex
      if (exclude.length()
          && StringTool::ereg(exclude, res.field_name(i), rl))
        continue;

      fields << (fields.str().length() ? "," : "") << res.field_name(i);
    } // for

    Result result(fields.str());
    for(resultSizeType i=0; i < res.num_rows(); i++) {
      Serialize s;
      for(resultSizeType j=0; j < res.num_fields(); j++) {
        StringTool::regexMatchListType rl;

        // skip any fields from our exclusion regex
        if (exclude.length()
            && StringTool::ereg(exclude, res.field_name(j), rl))
          continue;

        std::string value;
        res[i][j].to_string(value);
        s.add( value );
      } // for

      openframe::Row row(s.compile());
      result.add(row);
    } // for
    return result;
  } // DBI::convert
} // namespace openframe

#endif
