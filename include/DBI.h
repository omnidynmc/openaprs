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
 $Id: DB.h,v 1.1 2005/11/21 18:16:02 omni Exp $
 **************************************************************************/

#ifndef __OPENAPRS_DBI_H
#define __OPENAPRS_DBI_H

#include <string>
#include <map>

#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <mysql++.h>

#include "OpenAPRS_Abstract.h"
#include "PutMySQL.h"

#include "openaprs.h"
#include "openaprs_string.h"
#include "misc.h"

namespace openaprs {

  using namespace putmysql;
  using std::string;
  using std::map;
  using std::list;
  using std::pair;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

#define DBI_MIN(a, b)			((a) < (b) ? (a) : (b))
#define DBI_MAX(a, b)			((a) > (b) ? (a) : (b))

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class DBI : public OpenAPRS_Abstract {
  public:
    DBI();				// constructor
    virtual ~DBI();			// destructor

    /**********************
     ** Type Definitions **
     **********************/

    typedef map<string, string> resultMapType;
    typedef map<string, string> queryMapType;
    typedef mysqlpp::StoreQueryResult resultType;
    typedef mysqlpp::StoreQueryResult::size_type resultSizeType;
    typedef mysqlpp::Query queryType;

    class Set {
      public:
        resultMapType resultMap;
    }; // class Set


    typedef map<unsigned int, Set> setMapType;
    enum messageMatchEnum { TO, FROM, ALL };

    PutMySQL *handle() { return _sql; }
    const resultSizeType query(const string &, resultType &);
    void print(const string &, resultType &);
    void print(resultType &);
    void makeListFromField(const string &, resultType &, string &);
    static const size_t maxNewlineLen(const string &);
    static const bool lineForNewlineRow(const string &, string &, const size_t);

    /***************
     ** Variables **
     ***************/

    class Inject {
      public:
        Inject(DBI *, PutMySQL *, mysqlpp::Connection *);
        ~Inject();

        const bool getCallsignId(const string &, string &);
        const bool getPathId(const string &, string &);
        const bool getStatusId(const string &, string &);
        const bool insertCallsign(const string &, string &);
        const bool insertPath(const string &, const string &, string &);
        const bool insertPacket(const string &, string &);
        const bool insertStatus(const string &, const string &, string &);

      private:
        DBI::queryMapType _query;
        DBI *_dbi;
        PutMySQL *_sql;
        mysqlpp::Connection *_sqlpp;
    }; // class Inject

    class Create {
      public:
        Create(DBI *, PutMySQL *);
        ~Create();

        const unsigned int disableObjectBeacon(const string &);

        const bool Object(const string &,
                          const string &,
                          const string &,
                          const double,
                          const double,
                          const string &,
                          const string &,
                          const double,
                          const int,
                          const double,
                          const bool,
                          const int,
                          const string &,
                          const time_t,
                          const time_t,
                          const bool);
        const bool Position(const string &,
                            const string &,
                            const double,
                            const double,
                            const string &,
                            const string &,
                            const double,
                            const int,
                            const double,
                            const bool,
                            const int,
                            const string &,
                            const bool);
        const unsigned int getPendingMessages(setMapType &);
        const unsigned int setMessageSent(const int, const string &, const time_t);
        const unsigned int setMessageError(const int);
        const unsigned int getPendingPositions(setMapType &);
        const unsigned int setPositionSent(const int, const time_t);
        const unsigned int setPositionError(const int);
        const unsigned int getPendingObjects(const time_t, setMapType &);
        const unsigned int setObjectSent(const int, const string &, const time_t);
        const unsigned int setObjectError(const int);

      private:
        DBI *_dbi;
        PutMySQL *_sql;
    }; // class Create

    class Position {
      public:
        Position(DBI *, PutMySQL *);
        ~Position();

        const unsigned int deleteObject(const string &);

        const unsigned int LastByCallsign(const string &,
                                          const time_t,
                                          const time_t,
                                          const unsigned int,
                                          setMapType &);
        const unsigned int HistoryByCallsign(const string &,
                                             const time_t,
                                             const time_t,
                                             const unsigned int,
                                             setMapType &);
        const unsigned int LastByLatLong(const double,
                                         const double,
                                         const double,
                                         const double,
                                         const time_t,
                                         const time_t,
                                         const unsigned int,
                                         setMapType &);
      private:
        DBI *_dbi;
        PutMySQL *_sql;
    }; // class Position

    class Message {
      public:
        Message(DBI *, PutMySQL *);
        ~Message();

        const unsigned int LastByCallsign(const string &,
                                          const time_t,
                                          const time_t,
                                          const unsigned int,
                                          setMapType &);
      private:
        DBI *_dbi;
        PutMySQL *_sql;
    }; // class Message

    class Uls {
      public:
        Uls(DBI *, PutMySQL *);
        ~Uls();

        const unsigned int FindByCallsign(const string &,
                                          setMapType &);
      private:
        DBI *_dbi;
        PutMySQL *_sql;
    }; // class Uls

    class Signup {
      public:
        Signup(DBI *, PutMySQL *);
        ~Signup();

        const bool isKey(const string &, const string &, const string &, bool &);
        const bool isBanned(const string &);
        const bool isActivated(const string &);
        const bool isUniqueCall(const string &);
        const bool isUniqueEmail(const string &);
        const bool isUserById(const string &);
        const bool isIEUpgrade(const string &, const string &);
        const bool setActivated(const string &, const bool);
        const bool setVerified(const string &, const bool);
        const bool setIE(const string &, const bool);
        const bool deleteUser(const string &);
        const bool ResendActivation(const string &);
        const bool PasswordReset(const string &);
        const bool RegisterForNotifications(const string &, const string &, const string &, const string &);
        const bool Create(const string &,
                          const string &,
                          const string &,
                          const string &,
                          const string &,
                          const string &,
                          const unsigned int,
                          const bool);

        const unsigned int FindUser(const string &,
                                    setMapType &);

      private:
        DBI *_dbi;
        PutMySQL *_sql;
    }; // class Signup

    class APRSMail {
      public:
        APRSMail(DBI *, PutMySQL *);
        ~APRSMail();

        const unsigned int writeError(const int, const char *, ...);
        const unsigned int setRead(const int, const time_t);
        const unsigned int getPending(setMapType &);

        DBI *_dbi;
        PutMySQL *_sql;
    }; // class APRSMail

    class Weather {
      public:
        Weather(DBI *, PutMySQL *);
        ~Weather();

        const unsigned int LastByCallsign(const string &,
                                          const time_t,
                                          const time_t,
                                          const unsigned int,
                                          setMapType &);
        const unsigned int HistoryByCallsign(const string &,
                                             const time_t,
                                             const time_t,
                                             const unsigned int,
                                             setMapType &);
        const unsigned int LastByLatLong(const double,
                                         const double,
                                         const double,
                                         const double,
                                         const time_t,
                                         const time_t,
                                         const unsigned int,
                                         setMapType &);
      private:
        DBI *_dbi;
        PutMySQL *_sql;
    }; // class Weather

    class Telemetry {
      public:
        Telemetry(DBI *, PutMySQL *);
        ~Telemetry();

        const unsigned int LabelsByCallsign(const string &,
                                            const time_t,
                                            const time_t,
                                            const unsigned int,
                                            setMapType &);
        const unsigned int EqnsByCallsign(const string &,
                                          const time_t,
                                          const time_t,
                                          const unsigned int,
                                          setMapType &);
        const unsigned int UnitsByCallsign(const string &,
                                           const time_t,
                                           const time_t,
                                           const unsigned int,
                                           setMapType &);
        const unsigned int BitsByCallsign(const string &,
                                          const time_t,
                                          const time_t,
                                          const unsigned int,
                                          setMapType &);
        const unsigned int LastByCallsign(const string &,
                                          const time_t,
                                          const time_t,
                                          const unsigned int,
                                          setMapType &);
        const unsigned int HistoryByCallsign(const string &,
                                             const time_t,
                                             const time_t,
                                             const unsigned int,
                                             setMapType &);
      private:
        DBI *_dbi;
        PutMySQL *_sql;
    }; // class Telemetry

    class Extra {
      public:
        Extra(DBI *, PutMySQL *);
        ~Extra();

        const unsigned int FrequencyByCallsign(const string &,
                                               const time_t,
                                               const time_t,
                                               const unsigned int,
                                               setMapType &);

      private:
        DBI *_dbi;
        PutMySQL *_sql;
    }; // class Extra

    class Email {
      public:
        Email(DBI *, PutMySQL *);
        ~Email();

        const bool find(const string &,
                        const string &,
                        string &);
        const bool add(const string &,
                       const string &,
                       const string &);
        const bool remove(const string &,
                          const string &);

        const int list(const string &,
                       setMapType &);
        const bool ignore(const string &);
        const bool cache(const string &, const string &);
        const bool cached(const string &, const string &);

      private:
        DBI *_dbi;
        PutMySQL *_sql;
    }; // class Email

    /*************
     ** Members **
     *************/
    const unsigned int lastMessageID(const string &, string &);
    const unsigned int nextSendMessageID(const string &);
    const bool getMessageDecayID(const string &, const string &, const string &, string &);
    const unsigned int getMessagesByCallsign(const string &, const messageMatchEnum, const time_t, setMapType &);
    const bool getUserLogin(const string &, const string &, resultMapType &);
    const unsigned int updateUserLoginTS(const int);
    const bool getUserByCallsign(const string &, resultMapType &);
    const bool getUserOptions(const int, resultMapType &);
    const bool getObjectByName(const string &, resultMapType &);
    const unsigned int getMessages(const string &, messageMatchEnum, const time_t, resultMapType &);
    const bool doKillObject(const string &, const string &);
    const bool getObjectDecayID(const string &, string &);
    const bool createMessage(const string &, const string &, const string &, const bool);
    const bool createMessage(const string &, const string &, const string &, unsigned int, const bool);
    const bool registerMessageSession(const string &, const string &);
    const bool unregisterMessageSession(const string &, const string &);
    const unsigned int getLastMessageID(const string &, const string &);
    const unsigned int incLastMessageID(const string &, const string &);
    const unsigned int setMessageAck(const string &, const string &, const string &);
    const bool isUserSession(const string &, const time_t);
    const bool tryVerify(const string &, const string &, const string &, string &);
    const bool writeError(const string &);
    const string Wildcard(const string &);

    const unsigned int update(const string &);
    const unsigned int fetch(const string &query, setMapType &sm) { return fetchRows(query, sm); }
    const unsigned int fetch(const string &sc, const string &sq, setMapType &sm) { return fetchRowsAndCount(sc, sq, sm); }
    const unsigned int fetchRows(const string &, setMapType &);
    const unsigned int fetchRowsAndCount(const string &, const string &, setMapType &);
    const bool connect();
    const bool ping();

    /***************
     ** Variables **
     ***************/
    Create *create;
    Email *email;
    Inject *inject;
    Message *message;
    APRSMail *aprsmail;
    Position *position;
    Extra *extra;
    Signup *signup;
    Telemetry *telemetry;
    Uls *uls;
    Weather *weather;

  protected:
    mysqlpp::Connection *_sqlpp;
  private:
    PutMySQL *_sql;
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

}
#endif
