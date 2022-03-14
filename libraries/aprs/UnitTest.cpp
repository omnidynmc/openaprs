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
 $Id: APRS.cpp,v 1.2 2005/12/13 21:07:03 omni Exp $
 **************************************************************************/

#include <list>
#include <new>
#include <string>
#include <cstdarg>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

#include <dlfcn.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#include "APRS.h"
#include "StringToken.h"
#include "UnitTest.h"

namespace aprs {

using namespace std;

/**************************************************************************
 ** UnitTest Class                                                       **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

  UnitTest::UnitTest() : _ok(true) {
    // set defaults
  } // UnitTest::UnitTest

  UnitTest::~UnitTest() {
    // cleanup
  } // UnitTest::~UnitTest

  /***************
   ** Variables **
   ***************/

  const bool UnitTest::run() {
    _decodeUncompressedMoving();
    _decodeUncompressedMovingDfr();
    _decodeUncompressedMovingDfs();
    _decodeCompressed();
    _decodeCompressedMoving();
    _decodeWeather();
    _decodeMic_E();
    _decodeNmea_GPRMC();
    _decodeUltimeter();
    _decodeObject();
    _decodeMessage();
    _decodeTelemtry();
    _decodeDao();
    _decodeStatus();
    _decodeCapabilities();
    _decodeExperimental();
    _decodeItem();
    _decodePeetLogging();

    return ok();
  } // UnitTest::run

  void UnitTest::_decodeUncompressedMoving() {
    string packet;
    stringMapType stringMap;

    packet = _createPacketSentence("NV6G", "W6CO-5,OH7AA-1*,WIDE2-1,qAR,OH7AA", "!",
                                   "6253.52N/02739.47E>036/010",
                                   "this is a test",
                                   "/A=000465",
                                   stringMap);

    stringMap["aprs.packet.symbol.table"] = "/";
    stringMap["aprs.packet.symbol.code"] = ">";

    stringMap["aprs.packet.position.latitude"] = "62.892";
    stringMap["aprs.packet.position.longitude"] = "27.6578";
    stringMap["aprs.packet.position.ambiguity"] = "0";

    stringMap["aprs.packet.dirspd.speed"] = "18.52";
    stringMap["aprs.packet.dirspd.direction"] = "036";
    stringMap["aprs.packet.altitude"] = "141.73";

    _test(packet, stringMap, "basic uncompressed packet - moving target");
  } // UnitTest::_decodeUncompressedMoving

  void UnitTest::_decodeUncompressedMovingDfr() {
    string packet;
    stringMapType stringMap;

    packet = _createPacketSentence("NV6G", "W6CO-5,OH7AA-1*,WIDE2-1,qAR,OH7AA", "!",
                                   "6253.52N/02739.47E>"
                                   "036/010/293/876",
                                   "blah blah blah blah blah",
                                   stringMap);

    stringMap["aprs.packet.symbol.table"] = "/";
    stringMap["aprs.packet.symbol.code"] = ">";

    stringMap["aprs.packet.position.latitude"] = "62.892";
    stringMap["aprs.packet.position.longitude"] = "27.6578";
    stringMap["aprs.packet.position.ambiguity"] = "0";

    stringMap["aprs.packet.dirspd"] = "036/010";
    stringMap["aprs.packet.dirspd.speed"] = "18.52";
    stringMap["aprs.packet.dirspd.direction"] = "036";
    stringMap["aprs.packet.dirspd.range"] = "411.99";
    stringMap["aprs.packet.dirspd.hits"] = "8";
    stringMap["aprs.packet.dirspd.accuracy"] = "6";

    _test(packet, stringMap, "basic uncompressed packet - moving target w/ dfr");
  } // UnitTest::_decodeUncompressedMovingDfr

  void UnitTest::_decodeUncompressedMovingDfs() {
    string packet;
    stringMapType stringMap;

    packet = _createPacketSentence("NV6G", "W6CO-5,OH7AA-1*,WIDE2-1,qAR,OH7AA", "!",
                                   "6253.52N/02739.47E>"
                                   "DFS2374",
                                   "blah blah blah blah blah",
                                   stringMap);

    stringMap["aprs.packet.symbol.table"] = "/";
    stringMap["aprs.packet.symbol.code"] = ">";

    stringMap["aprs.packet.position.latitude"] = "62.892";
    stringMap["aprs.packet.position.longitude"] = "27.6578";
    stringMap["aprs.packet.position.ambiguity"] = "0";

    stringMap["aprs.packet.dfs.strength"] = "4";
    stringMap["aprs.packet.dfs.directivity"] = "180";
    stringMap["aprs.packet.dfs.gain"] = "5.01187";
    stringMap["aprs.packet.dfs.haat"] = "24.384";
    stringMap["aprs.packet.dfs.range"] = "20.3688";

    _test(packet, stringMap, "basic uncompressed packet - moving target w/ dfs");
  } // UnitTest::_decodeUncompressedMovingDfs

  void UnitTest::_decodeCompressed() {
    string packet;
    stringMapType stringMap;

    // ### BEGIN: Test 1 - Normal Compressed ###

    packet = _createPacketSentence("NV6G-2", "APRS,TCPIP*,qAC,FOURTH", "!",
                                   "I0-X;T_Wv&{-A",
                                   "igate testing",
                                   stringMap);

    stringMap["aprs.packet.symbol.table"] = "I";
    stringMap["aprs.packet.symbol.code"] = "&";

    stringMap["aprs.packet.position.latitude.decimal"] = "60.052010";
    stringMap["aprs.packet.position.longitude.decimal"] = "24.504507";

    _test(packet, stringMap, "decode compressed position - non moving");

    // ### BEGIN: Test 2 - Normal Compressed w/ WX###
    stringMap.clear();

    packet = _createPacketSentence("NV6G-2", "APRS,TCPIP*,qAC,FOURTH", "@",
                                   "011444z/:JF!T/W-_e!bg000t054r000p010P010h65b10073",
                                   "WS 2300 {UIV32N}",
                                   stringMap);

    stringMap["aprs.packet.symbol.table"] = "/";
    stringMap["aprs.packet.symbol.code"] = "_";

    stringMap["aprs.packet.position.latitude.decimal"] = "39.643333";
    stringMap["aprs.packet.position.longitude.decimal"] = "22.417168";

    stringMap["aprs.packet.weather.temperature.celcius"] = "12.22";
    stringMap["aprs.packet.weather.humidity"] = "65";
    stringMap["aprs.packet.weather.pressure"] = "1007.3";

    _test(packet, stringMap, "decode compressed position - w/ WX");

    // ### BEGIN: Test 3 - Normal Compressed w/ WX###
    stringMap.clear();

    // JA1UEX>APAGW,TCPIP*,qAC,T2JNET:@110857z3549.90N/13933.23E_000/000g000t047r000P000p000h38b10157 Fujimi-City,Saitama
    packet = _createPacketSentence("JA1UEX", "APAGW,TCPIP*,qAC,T2JNET", "@",
                                   "110857z3549.90N/13933.23E_000/000g000t047r000P000p000h38b10157",
                                   "Fujimi-City,Saitama",
                                   stringMap);

    stringMap["aprs.packet.symbol.table"] = "/";
    stringMap["aprs.packet.symbol.code"] = "_";

    stringMap["aprs.packet.position.latitude.decimal"] = "35.831665";
    stringMap["aprs.packet.position.longitude.decimal"] = "139.55383";

    stringMap["aprs.packet.weather.temperature.celcius"] = "8.33";
    stringMap["aprs.packet.weather.temperature.fahrenheit"] = "47";
    stringMap["aprs.packet.weather.humidity"] = "38";
    stringMap["aprs.packet.weather.pressure"] = "1015.7";

    stringMap["aprs.packet.weather.rain.24hour"] = "0.0";
    stringMap["aprs.packet.weather.rain.hour"] = "0.0";
    stringMap["aprs.packet.weather.rain.midnight"] = "0.0";

    _test(packet, stringMap, "decode compressed position - w/ WX");
  } // UnitTest::_decodeCompressed

  void UnitTest::_decodeWeather() {
    string packet;
    stringMapType stringMap;

    // ### BEGIN: Test 1 - Normal Compressed w/ WX###
    stringMap.clear();

    //CW5988>APRS,TCPXX*,qAX,CWOP-1:@100745z3745.57N/12225.87W_312/002g...t051r...p...P000h80b10196L000.DsVP
    packet = _createPacketSentence("CW5988", "APRS,TCPXX*,qAX,CWOP-1", "@",
                                   "100745z3745.57N/12225.87W_312/002g...t051r...p...P000h80b10196L000.DsVP",
                                   "",
                                   stringMap);

    stringMap["aprs.packet.symbol.table"] = "/";
    stringMap["aprs.packet.symbol.code"] = "_";

    stringMap["aprs.packet.position.latitude.decimal"] = "37.759499";
    stringMap["aprs.packet.position.longitude.decimal"] = "-122.4311";

    stringMap["aprs.packet.weather.temperature.celcius"] = "10.56";
    stringMap["aprs.packet.weather.temperature.fahrenheit"] = "51";
    stringMap["aprs.packet.weather.humidity"] = "80";
    stringMap["aprs.packet.weather.pressure"] = "1019.6";

    stringMap["aprs.packet.weather.wind.direction"] = "312";
    stringMap["aprs.packet.weather.wind.speed"] = "3.70";
//    stringMap["aprs.packet.weather.wind.gust"] = "0";

    _test(packet, stringMap, "decode uncompressed position - w/ WX and ...'s");

    // ### BEGIN: Test 2 - Normal Compressed w/ WX###
    stringMap.clear();

    //CW8221>APRS,APRS,TCPXX*,qAX,CWOP-2:CW8221>APRS,TCPXX*,qAX,CWOP-2:@101835z3748.53N/12225.25W_.../...g...t...P000h..b10196.DsVP
    packet = _createPacketSentence("CW8221", "APRS,TCPXX*,qAX,CWOP-1", "@",
                                   "101835z3748.53N/12225.25W_.../...g...t...P000h..b10196.DsVP",
                                   "",
                                   stringMap);

    stringMap["aprs.packet.symbol.table"] = "/";
    stringMap["aprs.packet.symbol.code"] = "_";

    stringMap["aprs.packet.position.latitude.decimal"] = "37.808834";
    stringMap["aprs.packet.position.longitude.decimal"] = "-122.4208";

//    stringMap["aprs.packet.weather.temperature.celcius"] = "-17.78";
//    stringMap["aprs.packet.weather.temperature.fahrenheit"] = "0";
//    stringMap["aprs.packet.weather.humidity"] = "80";
    stringMap["aprs.packet.weather.pressure"] = "1019.6";

//    stringMap["aprs.packet.weather.wind.gust"] = "0";

    _test(packet, stringMap, "decode uncompressed position - w/ WX and ...'s");

    // ### BEGIN: Test 3 - Positionless WX ###
    stringMap.clear();

    // VA3WWD-2>APTW01,VE3ZAP,qAR,VA3BAL-4:_02200420c015s000g000t033r000p000P000h97b.....tU2k
    packet = _createPacketSentence("VA3WWD-2", "APTW01,VE3ZAP,qAR,VA3BAL-4", "_",
                                   "02200420c015s000g000t033r000p000P000h97b.....tU2k",
                                   "",
                                   stringMap);

    stringMap["aprs.packet.weather.temperature.celcius"] = "0.56";
    stringMap["aprs.packet.weather.temperature.fahrenheit"] = "33";
    stringMap["aprs.packet.weather.humidity"] = "97";
    stringMap["aprs.packet.weather.pressure"] = "0.0";

    stringMap["aprs.packet.weather.wind.direction"] = "015";
    stringMap["aprs.packet.weather.wind.speed"] = "0.0";
    stringMap["aprs.packet.weather.wind.gust"] = "0";

    stringMap["aprs.packet.weather.rain.24hour"] = "0.0";
    stringMap["aprs.packet.weather.rain.hour"] = "0.0";
    stringMap["aprs.packet.weather.rain.midnight"] = "0.0";

    _test(packet, stringMap, "decode positionless weather");
  } // UnitTest::_decodeWeather

  void UnitTest::_decodeCapabilities() {
    string packet;
    stringMapType stringMap;

    // ### BEGIN: Test 1 - Normal Compressed w/ WX###
    stringMap.clear();

    // AD7AJ-11>APU25N,TCPIP*,qAC,T2SJC:<IGATE,MSG_CNT=6,LOC_CNT=1
    packet = _createPacketSentence("AD7AJ-11", "APU25N,TCPIP*,qAC,T2SJC", "<",
                                   "",
                                   "IGATE,MSG_CNT=6,LOC_CNT=1",
                                   stringMap);

    stringMap["aprs.packet.capabilities.IGATE"] = "undef";
    stringMap["aprs.packet.capabilities.LOC_CNT"] = "1";
    stringMap["aprs.packet.capabilities.MSG_CNT"] = "6";

    _test(packet, stringMap, "decode capabilities");
  } // UnitTest::_decodeCapbilities

  void UnitTest::_decodeExperimental() {
    string packet;
    stringMapType stringMap;

    // ### BEGIN: Test 1 - Normal Compressed w/ WX###
    stringMap.clear();

    // K0NY-2>APRS,WIDE2-1,qAR,KB0THN:{{d{{d2011-03-12 18:40:48,0.0,0.0,0,0.0,0.0,0,0.0,0.0,0,0,17,348,0,0,595,127,612,1023,1023,13.6,56
    packet = _createPacketSentence("AD7AJ-11", "APU25N,TCPIP*,qAC,T2SJC", "{{",
                                   "",
                                   "d{{d2011-03-12 18:40:48,0.0,0.0,0,0.0,0.0,0,0.0,0.0,0,0,17,348,0,0,595,127,612,1023,1023,13.6,56",
                                   stringMap);

    _test(packet, stringMap, "decode experimental");
  } // UnitTest::_decodeExperimental

  void UnitTest::_decodeItem() {
    string packet;
    stringMapType stringMap;

    // ### BEGIN: Test 1 - Normal Compressed w/ WX###
    stringMap.clear();

    // N5ROG-1>APWW07,WIDE1-1,WIDE2-1,qAC,N5ROG-1:)N5ROG-1!2930.78N/09522.71W#http://www.texasbigfoot.net/n5rog.html
    packet = _createPacketSentence("N5ROG-1", "APWW07,WIDE1-1,WIDE2-1,qAC,N5ROG-1", ")",
                                   "N5ROG-1!2930.78N/09522.71W#",
                                   "http://www.texasbigfoot.net/n5rog.html",
                                   stringMap);

    stringMap["aprs.packet.symbol.table"] = "/";
    stringMap["aprs.packet.symbol.code"] = "#";

    stringMap["aprs.packet.object.name"] = "N5ROG-1";
    stringMap["aprs.packet.object.type"] = "I";
    stringMap["aprs.packet.object.action"] = "!";

    stringMap["aprs.packet.position.latitude.decimal"] = "29.513000";
    stringMap["aprs.packet.position.longitude.decimal"] = "-95.37850";
    stringMap["aprs.packet.position.ambiguity"] = "0";

    _test(packet, stringMap, "decode item");
  } // UnitTest::_decodeItem

  void UnitTest::_decodeCompressedMoving() {
    string packet;
    stringMapType stringMap;

    packet = _createPacketSentence("KE6SAR-10", "APZMDR,WIDE3-2,qAo,OH2MQK-1", "!",
                                   "//zPHTfVv>!V_",
                                   "Tero, Green Volvo 960, GGL-880",
                                   stringMap);

    stringMap["aprs.packet.symbol.table"] = "/";
    stringMap["aprs.packet.symbol.code"] = ">";

    stringMap["aprs.packet.position.latitude.decimal"] = "60.358234";
    stringMap["aprs.packet.position.longitude.decimal"] = "24.808378";

    stringMap["aprs.packet.dirspd.speed"] = "57.23";
    stringMap["aprs.packet.dirspd.direction"] = "0";

    _test(packet, stringMap, "decode compressed position - moving");
  } // UnitTest::_decodeCompressedMoving

  void UnitTest::_decodeMic_E() {
    string packet;
    stringMapType stringMap;

    // ### BEGIN: Test 1 - Normal MIC_E ###

    packet = _createPacketSentence("KE6SAR-12", "SX15S6,TCPIP*,qAC,FOURTH", "'",
                                   "I',l \x1C>/]",
                                   "This is a test mic_e message",
                                   stringMap);

    stringMap["aprs.packet.symbol.table"] = "/";
    stringMap["aprs.packet.symbol.code"] = ">";

    stringMap["aprs.packet.position.latitude.decimal"] = "-38.25600";
    stringMap["aprs.packet.position.longitude.decimal"] = "145.18600";

    stringMap["aprs.packet.dirspd.speed"] = "0";
    stringMap["aprs.packet.dirspd.direction"] = "0";

    stringMap["aprs.packet.mic_e.raw.mbits"] = "110";

    _test(packet, stringMap, "decode mic_e");

    // ### BEGIN: Test 2 - Normal MIC_E ###
    stringMap.clear();

    packet = _createPacketSentence("KE6SAR-1", "TQ4W2V,WIDE2-1,qAo,W6CO", "`",
                                   "c51!f?>/]\"3x}",
                                   "=",
                                   stringMap);

    stringMap["aprs.packet.symbol.table"] = "/";
    stringMap["aprs.packet.symbol.code"] = ">";

    stringMap["aprs.packet.position.latitude.decimal"] = "41.787667";
    stringMap["aprs.packet.position.longitude.decimal"] = "-71.42016";

    stringMap["aprs.packet.dirspd.speed"] = "105.564";
    stringMap["aprs.packet.dirspd.direction"] = "35";

    stringMap["aprs.packet.mic_e.position.ambiguity"] = "0";
    stringMap["aprs.packet.mic_e.raw.mbits"] = "110";

    _test(packet, stringMap, "decode mic_e");
  } // UnitTest::_decodeMic_E

  void UnitTest::_decodeNmea_GPRMC() {
    string packet;
    stringMapType stringMap;

    packet = _createPacketSentence("KE6SAR-12", "APRS,WIDE2-1,qAR,WA4DSY", "$",
                                   "GPRMC,145526,A,3349.0378,N,08406.2617,W,23.726,27.9,121207,4.9,W*7A",
                                   "",
                                   stringMap);

    stringMap["aprs.packet.symbol.table"] = "/";
    stringMap["aprs.packet.symbol.code"] = "j";

    stringMap["aprs.packet.position.latitude.decimal"] = "33.8172";
    stringMap["aprs.packet.position.longitude.decimal"] = "-84.1043";

    _test(packet, stringMap, "decode nmea: GPRMC");
  } // UnitTest::_decodeNmea_GPRMC

  void UnitTest::_decodeUltimeter() {
    string packet;
    stringMapType stringMap;

    // ### BEGIN: Test 1 - Normal Ultimeter Packet###

    packet = _createPacketSentence("W6CO", "APN391,WIDE2-1,qAo,K2KZ-3", "$",
                                   "ULTW0053002D028D02FA2813000D87BD000103E8015703430010000C",
                                   "",
                                   stringMap);

    stringMap["aprs.packet.ultimeter.wind.direction"] = "64";
    //stringMap["aprs.packet.ultimeter.wind.speed"] = "8.3";	// FIXME: should be 0.3
    stringMap["aprs.packet.ultimeter.wind.gust"] = "8.3";	// FIXME: should be 2.3

    stringMap["aprs.packet.ultimeter.pressure"] = "1025.9";
    stringMap["aprs.packet.ultimeter.humidity"] = "100.0";
    stringMap["aprs.packet.ultimeter.temperature"] = "65.3";
    stringMap["aprs.packet.ultimeter.rain"] = "7.6";		// FIXME: rain.midnight and convert 4.1 metric

    _test(packet, stringMap, "decode ultimeter");

    // ### BEGIN: Test 2 - Ultimeter Packet Below 0F ###
    stringMap.clear();

    packet = _createPacketSentence("W6CO-2", "APN391,SQ2LYH-14,SR4DOS,WIDE2*,qAo,SR4NWO-1", "$",
                                   "ULTW00000000FFEA0000296F000A9663000103E80016025D",
                                   "",
                                   stringMap);

    stringMap["aprs.packet.ultimeter.wind.direction"] = "0";
    stringMap["aprs.packet.ultimeter.wind.gust"] = "0.0";

    stringMap["aprs.packet.ultimeter.pressure"] = "1060.7";
    stringMap["aprs.packet.ultimeter.humidity"] = "100.0";
    stringMap["aprs.packet.ultimeter.temperature"] = "6551.4";	// FIXME: should be -19.0
    stringMap["aprs.packet.ultimeter.rain"] = "0.0";		// FIXME: rain.midnight and convert 4.1 metric

    _test(packet, stringMap, "decode ultimeter - below 0F");

    return;

    // ### BEGIN: Test 3 - Ultimeter Packet Logging Format ###
    stringMap.clear();

    packet = _createPacketSentence("MB7DS", "APRS,TCPIP*,qAC,APRSUK2", "!",
                                   "!00000066013D000028710166--------0158053201200210",
                                   "",
                                   stringMap);

    stringMap["aprs.packet.ultimeter.wind.direction"] = "144";
    stringMap["aprs.packet.ultimeter.wind.gust"] = "14.7";

    stringMap["aprs.packet.ultimeter.pressure"] = "1035.3";
    stringMap["aprs.packet.ultimeter.temperature"] = "-0.2";
    stringMap["aprs.packet.ultimeter.temperature.in"] = "2.1";
    stringMap["aprs.packet.ultimeter.rain"] = "73.2";		// FIXME: rain.midnight and convert 4.1 metric

    _test(packet, stringMap, "decode ultimeter - below 0F");

    // ### BEGIN: Test 4 - Ultimeter Parser Bug? ###
    stringMap.clear();

    packet = _createPacketSentence("KD7DR-5", "APRS,WIDE2-1,qAR,KB7KFC-1", "$",
                                   "ULTW00000000031504ED2725000A9AA2000100EA007F04FB00000000",
                                   "",
                                   stringMap);

    stringMap["aprs.packet.ultimeter.wind.direction"] = "144";
    stringMap["aprs.packet.ultimeter.wind.gust"] = "14.7";

    stringMap["aprs.packet.ultimeter.pressure"] = "1035.3";
    stringMap["aprs.packet.ultimeter.temperature"] = "-0.2";
    stringMap["aprs.packet.ultimeter.temperature.in"] = "2.1";
    stringMap["aprs.packet.ultimeter.rain"] = "73.2";		// FIXME: rain.midnight and convert 4.1 metric

    _test(packet, stringMap, "decode ultimeter - below 0F");
  } // UnitTest::_decodeUltimeter

  void UnitTest::_decodeObject() {
    string packet;
    stringMapType stringMap;

    packet = _createPacketSentence("N6NAR", "APOA00,TCPIP*,qAC,N6NAR", ";",
                                   "OpenAPRS *051020z3746.22NO12226.26W&000/000",
                                   "",
                                   stringMap);

    stringMap["aprs.packet.symbol.table"] = "O";
    stringMap["aprs.packet.symbol.code"] = "&";

    stringMap["aprs.packet.object.action"] = "*";

    stringMap["aprs.packet.position.latitude.decimal"] = "37.770332";
    stringMap["aprs.packet.position.longitude.decimal"] = "-122.4376";

    _test(packet, stringMap, "decode object");
  } // UnitTest::_decodeObject

  void UnitTest::_decodeMessage() {
    string packet;
    stringMapType stringMap;
    const char *messageIds[] = { "1", "42", "10512", "a", "1Ff84", "F00b4", NULL };
    int i;

    for(i=0; messageIds[i] != NULL; i++) {
      stringMap.clear();
      packet = _createPacketSentence("NV6G", "APOA00,TCPIP*,qAC,N6NAR", ":",
                                     "N6NAR:",
                                     "",
                                     "Testing 1, 2, 3{" + string(messageIds[i]),
                                     stringMap);


      stringMap["aprs.packet.message.target"] = "N6NAR";
      stringMap["aprs.packet.message.text"] = "Testing 1, 2, 3";
      stringMap["aprs.packet.message.id"] = string(messageIds[i]);

      _test(packet, stringMap, "decode message: id - " + string(messageIds[i]));
    } // for
  } // UnitTest::_decodeMessage

  void UnitTest::_decodeTelemtry() {
    string packet;
    stringMapType stringMap;

    // there was 50.12 (now 501) there, should that actually be parsable?
    packet = _createPacketSentence("N6NAR", "APRS,TCPIP*,qAC,N6NAR", "T",
                                   "#324,000,038,257,255,50.12,01000001",
                                   "",
                                   stringMap);

    stringMap["aprs.packet.telemetry.analog0"] = "0.00";
    stringMap["aprs.packet.telemetry.analog1"] = "38.00";
    stringMap["aprs.packet.telemetry.analog2"] = "257.00";
    stringMap["aprs.packet.telemetry.analog3"] = "255.00";
    stringMap["aprs.packet.telemetry.analog4"] = "50.12";
    stringMap["aprs.packet.telemetry.digital"] = "01000001";
    stringMap["aprs.packet.telemetry.sequence"] = "324";

    _test(packet, stringMap, "decode telemetry");
  } // UnitTest::_decodeTelemtry

  void UnitTest::_decodeDao() {
    string packet;
    stringMapType stringMap;

    // ### BEGIN: Test 1 - Uncompressed Position with DAO ###

    // K0ELR-15>APOT02,WIDE1-1,WIDE2-1,qAo,K0ELR:/102033h4133.03NX09029.49Wv204/000!W33! 12.3V 21C/A=000665
    packet = _createPacketSentence("K0ELR-15", "APOT02,WIDE1-1,WIDE2-1,qAo,K0ELR", "/",
                                   "102033h4133.03NX09029.49Wv204/000!W33! ",
                                   "12.3V 21C",
                                   "/A=000665",
                                   stringMap);

    stringMap["aprs.packet.symbol.table"] = "X";
    stringMap["aprs.packet.symbol.code"] = "v";

    stringMap["aprs.packet.position.latitude.decimal"] = "41.550549";
    stringMap["aprs.packet.position.longitude.decimal"] = "-90.49155";
    stringMap["aprs.packet.position.ambiguity"] = "0";

    stringMap["aprs.packet.dirspd.speed"] = "0.00";
    stringMap["aprs.packet.dirspd.direction"] = "204";

    stringMap["aprs.packet.dao.datum"] = "W";

    _test(packet, stringMap, "basic uncompressed positon - w/ DAO");

    // ### BEGIN: Test 2 - Compressed Position with DAO ###
    stringMap.clear();

    packet = _createPacketSentence("K0ELR-15", "APZMDR,WIDE2-2,qAo,N6NAR", "!",
                                   "/0(yiTc5y>{2O ",
                                   "http://www.openaprs.net",
                                   "!w11!",
                                   stringMap);

    stringMap["aprs.packet.symbol.table"] = "/";
    stringMap["aprs.packet.symbol.code"] = ">";

    stringMap["aprs.packet.position.latitude.decimal"] = "60.152702"; 	// FIXME: Verifiy these
    stringMap["aprs.packet.position.longitude.decimal"] = "24.662191";	// FIXME: Verify these

    stringMap["aprs.packet.dao.datum"] = "w";

    _test(packet, stringMap, "basic compressed position - w/ WGS84 BASE91 DAO");

    // ### BEGIN: Test 3 - MIC_E Position with DAO ###
    stringMap.clear();

    // OH2JCQ-9>VP1U88,TRACE2-2,qAR,OH2RDK-5:'5'9\"^Rj/]\"4-}Foo !w66!Bar
    packet = _createPacketSentence("N6NAR-15", "VP1U88,TRACE2-2,qAR,OH2RDK-5", "'",
                                   "5'9\"^Rj/]\"4-}",
                                   "Foo Bar",
                                   "!w66!",
                                   stringMap);

    stringMap["aprs.packet.symbol.table"] = "/";
    stringMap["aprs.packet.symbol.code"] = "j";

    stringMap["aprs.packet.position.latitude.decimal"] = "60.264705"; 	// FIXME: Verifiy these
    stringMap["aprs.packet.position.longitude.decimal"] = "25.188205";	// FIXME: Verify these

    stringMap["aprs.packet.dao.datum"] = "w";

    _test(packet, stringMap, "basic mic_e - w/ WGS84 BASE91 DAO");
  } // UnitTest::_decodeDao

  void UnitTest::_decodeStatus() {
    string packet;
    stringMapType stringMap;

    // ### BEGIN: Test 1 - Status with Timestamp ###

    // IZ2TJH>APU25N,TCPIP*,qAC,IK2CHZ-11:>280843zUI-View32 V2.03
    packet = _createPacketSentence("IZ2TJH", "APU25N,TCPIP*,qAC,IK2CHZ-11", ">",
                                   "280843z",
                                   "UI-View32 V2.03",
                                   "",
                                   stringMap);

    stringMap["aprs.packet.timestamp.decoded"] = "280843";

    _test(packet, stringMap, "status with timestamp");
  } // UnitTest::_decodeStatus

  void UnitTest::_decodePeetLogging() {
    string packet;
    stringMapType stringMap;

    // ### BEGIN: Test 1 - Status with Timestamp ###

    // K4GIG-7>APZWX,AL2-2,qAR,N8DEU-5:!!0000006C02B40000----02F001B3----000100F900000062
    packet = _createPacketSentence("K4GIG-7", "APZWX,AL2-2,qAR,N8DEU-5", "!!",
                                   "",
                                   "0000006C02B40000----02F001B3----000100F900000062",
                                   "",
                                   stringMap);

    _test(packet, stringMap, "decode peetlogging");
  } // UnitTest::_decodePeetLogging

  const string UnitTest::_createPacketSentence(const string &source, const string &path, const string &command, const string &content,
                                               const string &comment, stringMapType &stringMap) {
    return _createPacketSentence(source, path, command, content, comment, "", stringMap);
  } // _createPacketSentence

  const string UnitTest::_createPacketSentence(const string &source, const string &path, const string &command, const string &content,
                                               const string &comment, const string &suffix, stringMapType &stringMap) {
    StringToken p;
    string packet;
    stringstream s;
    size_t i;

    p.setDelimiter(',');
    p = path;

    s << source
      << ">"
      << path
      << ":"
      << command
      << content
      << comment
      << suffix;

    packet = s.str();

    stringMap["aprs.packet.source"] = source;
    stringMap["aprs.packet.command"] = command;
    stringMap["aprs.packet.comment"] = comment;

    for(i=0; i < p.size(); i++) {
      s.str("");
      s << "aprs.packet.path" << i;
      stringMap[s.str()] = p[i];
    } // for

    return packet;
  } // UnitTest::_createPacketSentence

  const bool UnitTest::_test(const string &parseMe, stringMapType &stringMap, const string &testName) {
    APRS *a;
    bool isOK = true;
    stringMapType::iterator ptr;
    APRS::stringMapType::iterator ptr1;

    try {
      a = new APRS(parseMe, time(NULL));
    } // try
    catch(APRS_Exception e) {
      cout << " not ok - " << testName << " failed to parse: " << testName << "; " << e.message() << endl;
      ok(false);
      return false;
    } // catch

    for(ptr = stringMap.begin(); ptr != stringMap.end(); ptr++) {
      if (!a->isString(ptr->first)) {
        cout << " not ok - " << testName << " [key: " << ptr->first << "] is missing" << endl;
        isOK = false;
        continue;
      } // if

      if (ptr->second != a->getString(ptr->first)) {
        cout << " not ok - " << testName << " [key: " << ptr->first << "] (" << ptr->second << ") != (" << a->getString(ptr->first) << ")" << endl;
        isOK = false;
      } // if
    } // for

    if (isOK)
      cout << " ok - " << testName << endl;
    else {
      STRINGTOOL_DEBUG_STRINGS(a, stringList, "root");
      while(!stringList.empty()) {
        cout << stringList.front() << endl;
        stringList.pop_front();
      } // while
    } // else

    delete a;

    ok(isOK);

    return isOK;
  } // _test
} // namespace aprs
