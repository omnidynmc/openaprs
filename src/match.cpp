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
 $Id: match.cpp,v 1.1 2005/11/21 18:16:04 omni Exp $
 **************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cctype>

#include "config.h"
#include "match.h"

namespace openaprs {

const bool match(const char *check, const char *orig) {
  while (*check == '*' || tolower(*check) == tolower(*orig) || *check == '?') {
    if (*check == '*') {
      if (*++check) {
	while (*orig)
	  if (match(check, orig++))
	    return true;

	return false;
      } // if
      else {
	return true;
      } // else
    } // if
    else if (!*check)
      return true;
    else if (!*orig)
      return false;
    else {
      ++check;
      ++orig;
    } // if
  } // while

  return false;
} // match

}
