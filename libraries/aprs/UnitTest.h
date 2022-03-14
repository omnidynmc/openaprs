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
 $Id: APRS.h,v 1.1 2005/11/21 18:16:02 omni Exp $
 **************************************************************************/

#ifndef __LIBAPRS_UNITTEST_H
#define __LIBAPRS_UNITTEST_H

#include <string>
#include <map>

#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <regex.h>

namespace aprs {

  using std::string;
  using std::map;
  using std::list;
  using std::pair;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class UnitTest {
    public:
      UnitTest();				// constructor
      virtual ~UnitTest();			// destructor

      /**********************
       ** Type Definitions **
       **********************/
      typedef map<string, string> stringMapType;

      /*************
       ** Members **
       *************/
      const bool run();
      const bool ok() const { return _ok; }
      void ok(const bool ok) {
        if (ok)
          return;

        _ok = ok;
      } // ok

    protected:
      void _decodeUncompressedMoving();
      void _decodeUncompressedMovingDfr();
      void _decodeUncompressedMovingDfs();
      void _decodeCompressed();
      void _decodeCompressedMoving();
      void _decodeWeather();
      void _decodeMic_E();
      void _decodeNmea_GPRMC();
      void _decodeUltimeter();
      void _decodeObject();
      void _decodeMessage();
      void _decodeTelemtry();
      void _decodeDao();
      void _decodeStatus();
      void _decodeCapabilities();
      void _decodeExperimental();
      void _decodeItem();
      void _decodePeetLogging();

       const string _createPacketSentence(const string &, const string &, const string &, const string &,
                                          const string &, stringMapType &);

       const string _createPacketSentence(const string &, const string &, const string &, const string &,
                                          const string &, const string &, stringMapType &);

      const bool _test(const string &, stringMapType &, const string &);
    private:
      bool _ok;
  };

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // namespace aprs
#endif
