/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** pBurst, Internet Relay Chat Protocol Burster                         **
 ** Copyright (C) 1999 Gregory A. Carter                                 **
 **                    Daniel Robert Karrels99                           **
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
 $Id: Event.cpp,v 1.1.1.1 2003/08/17 01:16:12 omni Exp $
 **************************************************************************/

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>

#include <time.h>
#include <unistd.h>

#include "Event.h"

#include "openframe/ConfController.h"

namespace handler {
using openframe::ConfController;
using std::stringstream;
using std::string;

/**************************************************************************
 ** Event Class                                                            **
 **************************************************************************/

/**************************************
 ** Constructor / Destructor Members **
 **************************************/

Event::Event(const eventId addId, const string &addMatch, const unsigned int addFlags, 
             EventHandler *addEventHandler) {
  myId = addId;
  myMatch = addMatch;
  myEventHandler = addEventHandler;
  addFlag(addFlags);
} // Event::Event

Event::~Event() {
  closeEvent();
} // Event::~Event

/***************
 ** Variables **
 ***************/

const Event::flagType  Event::FLAG_CLEANUP		=		0x0001;

/*******************
 ** Event Members **
 *******************/

const bool Event::openEvent(const string &openFilename, ios_base::openmode openModes) {
  /*
   * 06/20/2003: Try and open a file stream from fileName.
   */

  if (myFile.is_open())
    return true;

  myFile.open(openFilename.c_str(), openModes);

  if (myFile.is_open())
    return true;

  return false;
} // Event::openEvent

const bool Event::closeEvent() {
  if (myFile.is_open() == true)
    myFile.close();

  return true;
} // Event::closeEvent

void Event::flushEvent() {
  if (myFile.is_open() == true)
    myFile.flush();
} // Event::flushEvent

const unsigned int Event::appendEvent(const string &appendMe) {
  if (myFile.is_open() == true)
    myFile << appendMe;

  return appendMe.length();
} // Event::appendEvent

const unsigned int Event::appendEvent(const stringstream &appendMe) {
  if (myFile.is_open() == true)
    myFile << appendMe.str();

  return 1;
} // Event::appendEvent

/**********************
 ** Operator Members **
 **********************/

Event &Event::operator<<(const char &appendMe) {
  if (myFile.is_open() == true)
    myFile << appendMe;
  return *this;
} // Event &Event::operator<<

Event &Event::operator<<(const string &appendMe) {
  if (myFile.is_open() == true)
    myFile << appendMe;
  return *this;
} // Event &Event::operator<<

Event &Event::operator<<(const int &appendMe) {
  if (myFile.is_open() == true)
    myFile << appendMe;
  return *this;
} // Event &Event::operator<<

Event &Event::operator<<(const unsigned int &appendMe) {
  if (myFile.is_open() == true)
    myFile << appendMe;
  return *this;
} // Event &Event::operator<<

Event &Event::operator<<(const long int &appendMe) {
  if (myFile.is_open() == true)
    myFile << appendMe;
  return *this;
} // Event &Event::operator<<

Event &Event::operator<<(const unsigned long int &appendMe) {
  if (myFile.is_open() == true)
    myFile << appendMe;
  return *this;
} // Event &Event::operator<<

Event &Event::operator<<(const short int &appendMe) {
  if (myFile.is_open() == true)
    myFile << appendMe;
  return *this;
} // Event &Event::operator<<

Event &Event::operator<<(const unsigned short int &appendMe) {
  if (myFile.is_open() == true)
    myFile << appendMe;
  return *this;
} // Event &Event::operator<<

Event &Event::operator<<(const stringstream &appendMe) {
  if (myFile.is_open() == true)
    myFile << appendMe.str();
  return *this;
} // Event &Event::operator<<

Event &Event::operator<<(const double &appendMe) {
  if (myFile.is_open() == true)
    myFile << appendMe;
  return *this;
} // Event &Event::operator<<

/**************************************************************************
 ** Event Class                                                          **
 **************************************************************************/

 /*
  * 06/20/2003: There will probably never be anything here for the event
  *             event class but just in case we'll leave it space. -GCARTER
  */
}
