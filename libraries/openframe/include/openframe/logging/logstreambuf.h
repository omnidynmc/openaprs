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

#ifndef __OPENFRAME_LOGSTREAMBUF_HH
#define __OPENFRAME_LOGSTREAMBUF_HH

#include <cstdio> // for EOF
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <locale>

#include <openframe/logging/loglevel.h>

namespace openframe {
  //we define here logstream and logstreambuf which should integrates well with iostream from STL

  /**
   * \class logstreambuf
   *
   * \brief This class defines an overloaded streambuf, to be used with logstream
   * It manages filtering in and out incoming messages from the stream.
   * It provides helpful function for derivated classes to implement prefix adding on output to sinks, on sync() calling...
   *
   * Date&Time (internally added) [TODO| ThreadID (internally added)] |Custom Prefix (set by logstream) | Message ( sent by logstream )
   */
  class logstreambuf: public std::stringbuf {
    protected :
      std::string ptm_prefix;
      loglevel::Level ptm_loglevel;

      bool ptm_logtime;

      //IDEA : logstreambuf is a string buffer. Therefore it can be accessed and manipulated by a stringstream if needed.
      //On flush, the content of the buffer is transferred to a sink ( derivated classes )

      //filter boolean
      bool ptm_filterin;

    public:
      logstreambuf();
      ~logstreambuf();

      //to manage prefix
      void resetprefix(const std::string& newprefix = 0);
      const std::string& getprefix() const;

      void resetlogtime(bool v=true) {
        ptm_logtime = v;
      } // resetlogtime

      //to start and stop getting messages
      void filterout() {
//std::cout << "filterout" << std::endl;
        ptm_filterin = false;
      } // filterout
      void filterin(loglevel::Level lvl) {
//std::cout << "filterin" << std::endl;
        if (!ptm_filterin) str(""); // remove any lingering kaka, like stupid newlines getting swindled in
        ptm_loglevel = lvl;
        ptm_filterin = true;
      } // filterin

    protected:
      ///Synchronize stream buffer
      ///must be overloaded by implementations of this abstract class (depending on log output)
      virtual int sync () = 0;

      std::string getlocaltime();
      std::string getloglevel();

      ///Output functions (put)
      ///Write sequence of characters
      virtual std::streamsize xsputn(const char * s, std::streamsize n);
      virtual std::streamsize sputn(const char * s, std::streamsize n);
      virtual int sputc(char c);
      ///Write character in the case of overflow ( endl for exemple )
      virtual int overflow(int c = EOF);
  }; // class logstreambuf

/* TODO
///output to syslog
class syslogstreambuf: public logstreambuf
{
    virtual int sync ( );
};

///output to win32dbg
class win32logstreambuf: public logstreambuf
{
    virtual int sync ( );
};
*/

  ///Output to clog
  class clogstreambuf : public logstreambuf {
      //clog as sink on sync()
      std::streambuf* ptm_clogbuf;
    public:
      clogstreambuf();
      ~clogstreambuf();

    protected :
      virtual int sync();
  }; // class clogstreambuf


  ///Output to clog
  class filelogstreambuf : public logstreambuf {
      //clog as sink on sync()
      std::filebuf ptm_filelogbuf;
    public:
      filelogstreambuf(const std::string &filename);
      ~filelogstreambuf();

    protected :
      virtual int sync();
  }; // class filelogstreambuf
}// openframe core
#endif // __OPENFRAME_LOGSTREAMBUF_H
