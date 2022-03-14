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

#ifndef __LIBOPENFRAME_DBI_H
#define __LIBOPENFRAME_DBI_H

#include <string>
#include <map>

#include <mysql++.h>

#include "OpenFrame_Abstract.h"
#include "Result.h"

#include "noCaseCompare.h"

namespace openframe {
  using std::string;
  using std::map;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

#define DBI_MAX(a, b)		(a > b ? a : b)

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class DBI_Exception : public OpenFrame_Exception {
    public:
      DBI_Exception(const string message) throw() : OpenFrame_Exception(message) {
      } // OpenAbstract_Exception

   private:
  }; // class DBI_Exception


  class DBI : public OpenFrame_Abstract {
    public:
      DBI(const string &, const string &, const string &, const string &);
      virtual ~DBI();

      static const size_t DEFAULT_WRAP;

      // ### Type Definitions ###
      typedef mysqlpp::StoreQueryResult::list_type::size_type resultSizeType;
      typedef mysqlpp::StoreQueryResult resultType;
      typedef unsigned long long simpleResultSizeType;
      typedef mysqlpp::SimpleResult simpleResultType;
      typedef mysqlpp::Query queryType;
      typedef map<string, string> queryMapType;
      typedef vector<resultType> combineResultType;
      typedef combineResultType::size_type combineResultSizeType;
      typedef map<string, Result, noCaseCompare> storeType;
      typedef storeType::size_type storeSizeType;

      mysqlpp::Connection *handle() { return _sqlpp; }

      static void print(resultType &res);
      static void print(resultType &res, const size_t);
      static void print(resultType &res, const size_t, string &);
      static void print(const string &, resultType &res);
      static void print(const string &, resultType &res, string &);
      static void makeListFromField(const string &, resultType &, string &);
      static void makeListFromField(const string &, const string &, resultType &, string &);
      static const string safe(const string &);
      static const bool lineForNewlineRow(const string &, string &, const size_t);
      static const size_t maxNewlineLen(const string &);
      static const string wordwrap(const string &, const size_t);
      static void combine(resultType &, resultType &, openframe::Result &);
      static void combine(combineResultType &, openframe::Result &);
      static const bool diff(resultType &, resultType &);
      static const bool diff(resultType &, resultType &, const string &, const bool);
      static const bool diffset(resultType &, resultType &, const string &, const bool);
      const resultSizeType query(const string &, resultType &);

      // stored results
      void store(const string &, resultType &);
      const bool restore(const string &, resultType &);
      void store(const string &, Result &);
      const bool restore(const string &, Result &);
      const bool purge(const string &);
      const bool save(const string &, const string &name = "");
      const bool load(const string &);
      const storeSizeType store(storeType &);
      const storeType::const_iterator storeBegin() const { return _store.begin(); }
      const storeType::const_iterator storeEnd() const { return _store.end(); }
      static Result convert(resultType &, const string &exclude="");

    protected:
      // ### Protected Members ###
      const bool _connect();

      // ### Protected Variables ###
      mysqlpp::Connection *_sqlpp;
      storeType _store;
      string _db;
      string _host;
      string _user;
      string _pass;

    private:

  }; // OpenFrame_Abstract

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // namespace openframe

#endif
