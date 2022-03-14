/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** HAL9000, Internet Relay Chat Bot                                     **
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
 $Id: Symbols.cpp,v 1.12 2003/09/05 22:23:41 omni Exp $
 **************************************************************************/

#include <fstream>
#include <string>
#include <queue>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <list>
#include <map>
#include <new>
#include <iostream>
#include <fstream>
#include <sstream>

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <netdb.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>

#include "Symbols.h"

namespace dcc {

  using namespace std;

/**************************************************************************
 ** Symbols Class                                                        **
 **************************************************************************/

  Symbols::Symbols() {
    symbolMap.clear();

    add("car", "/", ">");
    add("truck", "/", "k");
    add("van", "/", "v");
    add("jogger", "/", "[");
    add("helicopter", "/", "X");
    add("plane", "/", "'");
    add("rv", "/", "R");
    add("motorcycle", "/", "<");

    return;
  } // Symbols::Symbols

  Symbols::~Symbols() {
    return;
  } // Symbols::~Symbols

  const bool Symbols::add(const string &name,
                          const string &table,
                          const string &code) {
    symbolMapType::iterator ptr;
    Symbol sy;

    if ((ptr = symbolMap.find(name)) != symbolMap.end())
      return false;

    sy.name = name;
    sy.table = table;
    sy.code = code;

    symbolMap.insert(pair<string, Symbol>(name, sy));

    return true;
  } // Symbols::add

  const bool Symbols::remove(const string &name) {
    symbolMapType::iterator ptr;

    if ((ptr = symbolMap.find(name)) == symbolMap.end())
      return false;

    symbolMap.erase(ptr);

    return true;
  } // Symbols::remove

  const unsigned int Symbols::clear() {
    unsigned int numRemoved;			// number of users removed

    // initialize variables
    numRemoved = symbolMap.size();

    // clear client list
    symbolMap.clear();

    return numRemoved;
  } // Symbols:clear

  const bool Symbols::find(const string &name, Symbol &sy) {
    symbolMapType::iterator ptr;

    if ((ptr = symbolMap.find(name)) == symbolMap.end())
      return false;

    sy = ptr->second;

    return true;
  } // Symbols::find

} // namespace dcc
