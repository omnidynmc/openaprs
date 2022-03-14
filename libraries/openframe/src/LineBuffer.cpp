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

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <new>
#include <string>

#include "openframe/LineBuffer.h"

namespace openframe {

using std::string;

LineBuffer::LineBuffer(char delimiter) {
  // create new string
  delim = delimiter;
}

LineBuffer::~LineBuffer() {
}

void LineBuffer::add(const string &addMe) {

  // append line to buffer
  buf.append(addMe);

  return;
}

bool LineBuffer::remove(int length) {
  string::size_type len = buf.size();

  if (length <= 0)
    return false;
 
  if ((string::size_type) length >= len) {
    buf = "";
    return false;
  }

  buf.erase(0, length);

  return true;
}

bool LineBuffer::readLine(string &retMe) {
  string::size_type pos;		// position of string

  // remove new lines
  while(this->length() != 0 && buf[0] == '\n')
    buf.erase(buf.begin());

  // return false is nothing is left
  if (this->length() == 0)
    return false;

  // find the first ``delim'' character
  pos = buf.find(delim);
  if (string::npos == pos)
    return false;

  retMe = buf.substr(0, pos + 1);
  buf.erase(0, pos + 1);

  return true;
}

const int LineBuffer::dumpBuffer(string &setMe) {
  setMe = buf;

  buf.erase(buf.begin(), buf.end());

  return setMe.length();
} // LineBuffer::dumpBuffer

void LineBuffer::flush() {

  // clear out data
  buf.erase(buf.begin(), buf.end());

  return;
} // LineBuffer::flush

LineBuffer & LineBuffer::operator+=(const char *addMe) {
  this->add(addMe);
  return *this;
}


LineBuffer & LineBuffer::operator+=(const string &addMe) {
  this->add(addMe);
  return *this;
}

LineBuffer & LineBuffer::operator+=(const LineBuffer &aBuf) {
  buf += aBuf.buf;
  return *this;
}

LineBuffer & LineBuffer::operator=(const string &replaceWithMe) {
  buf = replaceWithMe;
  return *this;
}

LineBuffer & LineBuffer::operator=(const char *replaceWithMe) {
  buf = replaceWithMe;
  return *this;
}

LineBuffer & LineBuffer::operator=(const LineBuffer &aBuf) {
  buf = aBuf.buf;
  return *this;
}

}
