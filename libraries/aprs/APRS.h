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

#ifndef __LIBAPRS_APRS_H
#define __LIBAPRS_APRS_H

#include <string>
#include <sstream>
#include <map>
#include <exception>

#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "StringTool.h"
#include "UnitTest.h"

namespace aprs {

using namespace std;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class APRS_Exception : public std::exception {
  public:
    friend class UnitTest;

    APRS_Exception(const string message) throw() {
      if (!message.length())
        _message = "An unknown exception occured.";
      else
        _message = message;
    } // APRS_Exception

    virtual ~APRS_Exception() throw() { }
    virtual const char *what() const throw() { return _message.c_str(); }

    const char *message() const throw() { return _message.c_str(); }

  private:
    string _message;                    // Message of the exception error.
}; // class APRS_Exception

class APRS : public StringTool {
  public:
    APRS(const string &, const time_t);		// constructor
    virtual ~APRS();				// destructor

    // ### Type Definitions ###
    enum aprsPacketType {
      APRS_PACKET_POSITION		 	= 0,
      APRS_PACKET_MESSAGE 			= 1,
      APRS_PACKET_TELEMETRY			= 2,
      APRS_PACKET_STATUS			= 3,
      APRS_PACKET_CAPABILITIES			= 4,
      APRS_PACKET_WEATHER			= 5,
      APRS_PACKET_DX				= 6,
      APRS_PACKET_ITEM				= 7,
      APRS_PACKET_EXPERIMENTAL			= 8,
      APRS_PACKET_MAIDENHEAD			= 9,
      APRS_PACKET_PEETLOGGING			= 10,
      APRS_PACKET_BEACON			= 11,
      APRS_PACKET_UNKNOWN			= 254
    };

    // ### Members ###
    static const bool isValidLatLong(const double, const double);
    static const bool isValidMessage(const string &);
    static const bool isValidCallsign(const string &);
    static const string extractCallsign(const string &callsign) { return _extractCallsign(callsign); }
    static const bool calcDestPoint(const int, const double, const double, const double,
                              double &, double &);
    static const double calcDistance(double, double, double, double, char);
    static const double calcSpeed(const double, const time_t,
                               const int, const int);
    static double deg2rad(double);
    static double rad2deg(double);

    const double latitude() const { return _latitude; }
    const double longitude() const { return _longitude; }
    const double lat() const { return _latitude; }
    const double lng() const { return _longitude; }
    const char command() const { return _command; }
    const string source() const { return _source; }
    const string path() const { return _path; }
    const string status() const { return _status; }
    const string packet() const { return _packet; }
    const string body() const { return _body; }
    const time_t timestamp() const { return _timestamp; }
    const string id() const { return _id; }
    const bool position() const { return _position; }
    void position(const bool position) { _position = position; }
    const aprsPacketType packetType() const { return _packetType; }
    const bool parse() { return _parse(); }

    inline const char symbolTable() const { return _symbolTable; }
    inline const char symbolCode() const { return _symbolCode; }
    // ### Variables ###

  protected:
    // ### Members ###
    const bool _parse();
    const bool _parsePosition(const string &);
    const bool _parseMessage(const string &);
    const bool _parseNmea(const string &);
    const bool _parseTelemetry(const string &);
    const bool _parseMIC_E(const string &);
    const bool _parseStatus(const string &);
    const bool _parseCapabilities(const string &);
    const bool _parseExperimental(const string &);
    const bool _parsePeetLogging(const string &);
    const bool _parseBeacon(const string &);

    const bool _parseUltimeter(string &);
    const bool _parseWeather(string &);
    const bool _parseMessageTelemetry(string &);

    const bool _convertPos(const string &, const string &, const string &, const string &,
                           map<string, string> &);
    const bool _convertMIC_EToDecimal(string &);

    const bool _extractTimestamp(string &);
    const bool _extractWeatherTimestamp(string &);
    const bool _extractTelemetry(string &);
    const bool _extractNmea(string &);
    const bool _extractLocation(string &);
    const bool _extractLocationCompressed(string &);
    const bool _extractAltitude(string &);
    const bool _extractAltitudeHelper(string &, const std::string::size_type offset);
    const bool _extractDfs(string &);
    const bool _extractDfsHelper(string &, const std::string::size_type offset);
    const bool _extractAfrs(string &);
    const bool _extractAreaObject(string &);
    const bool _extractDfr(string &);
    const bool _extractDfrHelper(string &, const std::string::size_type offset);
    const bool _extractDirSpd(string &);
    const bool _extractWindDirSpd(string &);
    const bool _extractGustTemp(string &);
    const bool _extractRng(string &);
    const bool _extractAltSymbol(string &);
    const bool _extractSignpost(string &);
    const bool _extractPhg(string &);
    const bool _extractItem(string &);
    const bool _extractDao(string &, float &, float &);
    static const string _extractCallsign(const string &);

    const string _cleanupComment(const string &);
    const string _makeBufferSafe(const string &);

    void _symbol(const char symbolTable, const char symbolCode) {
      stringstream s;
      s.str("");
      s << symbolTable;
      addString("aprs.packet.symbol.table", s.str());
      s.str("");
      s << symbolCode;
      addString("aprs.packet.symbol.code", s.str());

      _symbolTable = symbolTable;
      _symbolCode = symbolCode;
    } // _symbol

    void _symbol(const string symbolTable, const string symbolCode) {
      assert(symbolTable.length() || symbolCode.length());	// bug

      addString("aprs.packet.symbol.table", string(symbolTable));
      addString("aprs.packet.symbol.code", string(symbolCode));

      _symbolTable = symbolTable[0];
      _symbolCode = symbolCode[0];
    } // _symbol

    void status(const string &status) {
      _status = status;
      replaceString("aprs.packet.comment", status);
    } // status

    // ### Members ###
    void _latlng(const double, const double);

    void _latlng(const string &lat, const string &lng) {
      _position = true;

      _latitude = atof(lat.c_str());
      _longitude = atof(lng.c_str());

      addString("aprs.packet.position.latitude.decimal", lat);
      addString("aprs.packet.position.longitude.decimal", lng);
    } // _latlng

  private:
    // ### Variables ###
    bool _position;			// has position data
    bool _telemetry;			// has telemtry data
    bool _weather;			// has weather data
    char _command;			// command
    char _symbolTable;			// symbol table
    char _symbolCode;			// symbol code
    string _packet;			// full packet string
    string _path;
    string _source;			// source of packet
    string _status;			// status report
    string _body;			// info portion of packet
    string _id;				// unique md5 id
    string _error;			// error message for throw
    double _latitude;			// latitude
    double _longitude;			// longitude
    aprsPacketType _packetType;		// type of aprs packet
    time_t _timestamp;			// timestamp
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

#define KnotToKmh(x)			(x * 1.852)
#define KphToKnot(x)			(x * 0.539956803)
#define	MphToKmh(x)			(x * 1.609344)
#define KmhToMs(x)			(x * 10 / 36)
#define MphToMs(x)			(x * 1.609344 * 10 / 36)
#define HInchToMm(x)			(x * 0.254)
#define FtToM(x)			(x * 0.3048);

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

int axtoi(const char *);

}
#endif
