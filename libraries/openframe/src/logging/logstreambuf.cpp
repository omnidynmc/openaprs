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

#include <cstring>
#include <stdexcept>
#include <iostream>
#include <sstream>

#include <openframe/logging/logstreambuf.h>

namespace openframe {
  logstreambuf::logstreambuf()
        : std::stringbuf(),//stringbuf in and out required
        ptm_prefix(""), //default no prefix
        ptm_logtime(true),
        ptm_filterin(false) {
  } // logstreambuf::logstreambuf

  logstreambuf::~logstreambuf() {
  } // logstreambuf::~logstreambuf


  void logstreambuf::resetprefix(const std::string &newprefix) {
    ptm_prefix = newprefix;
  } // logstreambuf::resetprefix

  const std::string& logstreambuf::getprefix() const {
    return ptm_prefix;
  } // logstreambuf::getprefix

  std::string logstreambuf::getlocaltime() {
    //TODO : insert useful prefixes
    //maybe distinction date / time useless
    char timebuf[32];
    if (ptm_logtime) {
        time_t rawtime;
        struct tm * timeinfo;

        time(&rawtime);
        timeinfo = localtime (&rawtime); //TODO Windows : localtime_s
        //TOFIX : strftime fails on windows... (output empty)
        strftime(timebuf,32,"[%m-%d-%Y %X %z] ",timeinfo);
    } // if
    return std::string(timebuf);
  } // logstreambuf::getlocaltime

  std::string logstreambuf::getloglevel() {
    std::stringstream s;
    s << std::right << std::setw(6) << ptm_loglevel << ": ";
    return s.str();
  } // logstreambuf::getloglevel

  ///Output functions (put)
  ///Write sequence of characters
  std::streamsize logstreambuf::sputn(const char *s, std::streamsize n) {
    std::streamsize ressize = 0;
//std::string blah;
//blah.assign(s, n);
//std::cout << "SPUTN(" << blah << ") filterin(" << ptm_filterin << ")" << std::endl;
    if (ptm_filterin) {
        char *laststr = (char *) s;
        //last string
        ressize += std::stringbuf::sputn(laststr, n);
    } // if
    else // filterout
      // fake a proper sputn.
      ressize += n;
    return ressize; //ressize == 0 means something is wrong -> will set ios::failbit
  } // logstreambuf::sputn

  std::streamsize logstreambuf::xsputn (const char *s, std::streamsize n) {
    std::streamsize ressize = 0;
//std::string blah;
//blah.assign(s, n);
//std::cout << "XSPUTN(" << blah << ") filterin(" << ptm_filterin << ")" << std::endl;
    if (ptm_filterin) {
        char *laststr = (char *) s;
        //last string
        ressize += std::stringbuf::xsputn(laststr, n);
    } // if
    else // filterout
      // fake a proper sputn.
      ressize += n;
    return ressize; //ressize == 0 means something is wrong -> will set ios::failbit
  } // logstreambuf::xsputn

  int logstreambuf::sputc(char c) {
    int ressize = 0;
//std::cout << "LOGLEVEL(" << ptm_loglevel << ") " << "SPUTC(" << c << ") filterin(" << ptm_filterin << ")" << std::endl;
    if (ptm_filterin) {
        ressize += std::stringbuf::sputc(c);
    } // if
    else // filterout
      // fake a proper sputn.
      ressize += 1;
    return ressize; //ressize == 0 means something is wrong -> will set ios::failbit
  } // logstreambuf::sputc

  int logstreambuf::overflow(int c) {
    int res = 0;
//std::cout << "LOGLEVEL(" << ptm_loglevel << ") " << "OVERFLOW(" << c << ") filterin(" << ptm_filterin << ")" << std::endl;
    if (ptm_filterin) {
      res = std::stringbuf::overflow(c);
      //TOTHINK ABOUT : we can do it here instead of in sync... if we want it into the stringbuf...
      //            std::string timestr = getlocaltime();
      //    sputn(timestr.c_str(),timestr.length());
    } // if
    else
      res = 0; //nothing to do, message filtered out
    return res;
  } // logstreambuf::overflow

/***************** For clogstreambuff : to output to clog *******/

  clogstreambuf::clogstreambuf() :  logstreambuf(), ptm_clogbuf(std::clog.rdbuf()) {
    //using clog as sink
  } // clogstreambuf::clogstreambuf

  clogstreambuf::~clogstreambuf() {
  } // clogstreambuf::~clogstreambuf

  ///Synchronizes (flush) the stream buffer
  int clogstreambuf::sync() {
    int res=0;

    //we add prefix only if we are in filterin state
    if (ptm_filterin) {
      if (ptm_logtime) {
        //TODO : speed up with fixed string size
        //passon prefixes strings
        std::string timestr = getlocaltime();
        ptm_clogbuf->sputn(timestr.data(), timestr.length());
      } // if

      ptm_clogbuf->sputn(ptm_prefix.data(), ptm_prefix.length());

      //transfer ptm_buf to clogbuf
      //copy character one by one...
      //maybe better to lock buffer, and get it whole at once...
      char c = logstreambuf::sbumpc();
      while (c != EOF) {
        ptm_clogbuf->sputc(c);
        c = logstreambuf::sbumpc();
      } // while
    } // if

    res += ptm_clogbuf->pubsync();
    return res;
  } // clogstreambuf::sync


/***************** For filelogstreambuf : to output to file *******/
  //using clog as sink
  filelogstreambuf::filelogstreambuf(const std::string & filename) : logstreambuf(), ptm_filelogbuf() {
    //TOTHINK ABOUT : do we delete the file everytime we open it ?
    //or provide an option bool append for example ?
    if (0 == ptm_filelogbuf.open(filename.c_str(), std::ios::out | std::ios::app) )
        throw std::logic_error("unable to open" + filename);
  } // filelogstreambuf::filelogstreambuf

  filelogstreambuf::~filelogstreambuf() {
    ptm_filelogbuf.close();
  } // filelogstreambuf::~filelogstreambuf

  ///Synchronizes (flush) the stream buffer
  int filelogstreambuf::sync() {
    int res=0;
    if (ptm_filelogbuf.is_open()) {
      //we add prefix only if we are in filterin state
      if (ptm_filterin) {
        //transfer ptm_buf to clogbuf
        //copy character one by one...
        //maybe better to lock buffer, and get it whole at once...
        if (ptm_logtime) {
          std::string timestr = getlocaltime();
          std::string loglevel = getloglevel();
          ptm_filelogbuf.sputn(timestr.data(), timestr.length());
          ptm_filelogbuf.sputn(loglevel.data(), loglevel.length());
        } // if

        char c = sbumpc();
        while (c != EOF) {
//if (c == '\n') std::cout << "\\n" << std::endl;
//else std::cout << c;
          ptm_filelogbuf.sputc(c);

          if (c == '\n' && ptm_logtime) {
            c = sbumpc();
            if (c != EOF) {
              std::string timestr = getlocaltime();
              std::string loglevel = getloglevel();
              ptm_filelogbuf.sputn(timestr.data(), timestr.length());
              ptm_filelogbuf.sputn(loglevel.data(), loglevel.length());
//              ptm_filelogbuf.sputn("        ", 8);
            } // if
          } // if
          else
            c = sbumpc();
        } // while
      } // if

      res += ptm_filelogbuf.pubsync();
    } // if
    return res;
  } // filelogstreambuf::sync
} // namespace openframe
