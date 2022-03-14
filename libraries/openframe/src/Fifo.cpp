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

#include <list>
#include <fstream>
#include <queue>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <new>
#include <iostream>
#include <string>
#include <exception>
#include <ctime>
#include <sstream>

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include <math.h>

#include "openframe/StringToken.h"
#include "openframe/StringTool.h"
#include "openframe/Fifo.h"

namespace openframe {

  /**************************************************************************
   ** Fifo Class                                                           **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  Fifo::Fifo(const fifo_st res_arg) {
    reserve(res_arg);
    _pos = begin();
  } // Fifo::Fifo

  Fifo::~Fifo() {
  } // Fifo::~Fifo

  Fifo &Fifo::add_charbuf(const char *buf, const size_t len) {
    for(size_t i=0; i < len; i++) {
      if (buf[i] == '\r') continue;	// skip linefeeds
      push_back(buf[i]);
    } // for
    _pos = begin();
    return *this;
  } // Fifo::add_charbuf

  bool Fifo::next_line(std::string &ret) {
    return next_delim(ret, '\n');
  } // Fifo::next_line

  bool Fifo::next_cstring(std::string &ret) {
    return next_delim(ret, '\0');
  } // Fifo::next_cstring

  bool Fifo::next_delim(std::string &ret, const char delim) {
    if (empty() ) return false;

    _pos = begin();
    bool found = false;
    while( _pos != end() ) {
      if (*_pos == delim) {
        found = true;
        break;
      } // if
      ret.append(1, *_pos);
      _pos++;
    } // while

    if (!found) return false;

    _pos++; // move off delim
    erase_to_pos();
    return true;
  } // Fifo::next_delim

  bool Fifo::next_bytes(const fifo_st len, std::string &ret) {
    fifo_itr _pos = begin();

    if (empty() || len > size() ) return false;

    fifo_st i;
    for(i=0;_pos != end() && i < len; i++) {
      ret.append(1, *_pos);
      _pos++;
    } // while

    if (i != len) return false;

    erase_to_pos();
    return true;
  } // Fifo::next_bytes

  void Fifo::erase_to_pos() {
    if (empty() ) return;

    erase(begin(), _pos);
    _pos = begin();
  } // Fifo::erase_to_pos

  void Fifo::clear() {
    fifo_t::clear();
  } // Fifo::clear
} // namespace openframe
