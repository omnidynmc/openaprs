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

#ifndef __LIBOPENFRAME_OPENFRAME_LOG
#define __LIBOPENFRAME_OPENFRAME_LOG

#include <iostream>
#include <iomanip>
#include <string>

#include <openframe/Refcount.h>

namespace openframe {

  class OpenFrame_Log : public Refcount {
    public:
      OpenFrame_Log(const std::string &ident) : _ident(ident) { _tid = pthread_self(); }
      virtual ~OpenFrame_Log() { }

      virtual const string ident() const { return _ident; }

      virtual void log(const std::string &message) {
        std::cout << message << std::endl;
      } // log

      virtual void debug(const std::string &message) {
        std::cout << message << std::endl;
      } // log

      virtual void info(const std::string &message) {
        std::cout << message << std::endl;
      } // log

      virtual void notice(const std::string &message) {
        std::cout << message << std::endl;
      } // notice

      virtual void warn(const std::string &message) {
        std::cout << message << std::endl;
      } // warn

      virtual void error(const std::string &message) {
        std::cout << message << std::endl;
      } // error

      virtual void alert(const std::string &message) {
        std::cout << message << std::endl;
      } // alert

      virtual void crit(const std::string &message) {
        std::cout << message << std::endl;
      } // crit

      virtual void emerg(const std::string &message) {
        std::cout << message << std::endl;
      } // emerg

      virtual void console(const std::string &message) {
        std::cout << message << std::endl;
      } // console

    protected:
      pthread_t _tid;
      std::string _ident;

    private:

  }; // OpenFrame_Log
} // namespace openframe

#endif
