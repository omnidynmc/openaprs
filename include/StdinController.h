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
 $Id: Server.h,v 1.1 2005/11/21 18:16:03 omni Exp $
 **************************************************************************/

#ifndef __OPENAPRS_STDINCONTROLLER_H
#define __OPENAPRS_STDINCONTROLLER_H

#include <string>

#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>
#include <form.h>

#include "OpenAPRS_Abstract.h"

namespace openaprs {
  using std::string;
  using std::map;
  using std::list;
  using std::pair;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class StdinCoord {
    public:
      StdinCoord(const int height, const int width, const int y, const int x) :
        _height(height), _width(width), _y(y), _x(x) { }
      ~StdinCoord() { }

      const int y() const { return _y; }
      const int x() const { return _x; }
      const int height() const { return _height; }
      const int width() const { return _width; }

    protected:
      int _height;
      int _width;
      int _y;
      int _x;
    private:
  };

  class StdinController_Exception : public OpenAPRSAbstract_Exception {
    public:
      StdinController_Exception(const string message) throw() : OpenAPRSAbstract_Exception(message) {
      } // StdinController_Exception

    private:
  }; // class StdinController_Exception

  class StdinController : public OpenAPRS_Abstract {
    public:
      StdinController(const string &, const int);
      virtual ~StdinController();

      // ### Type Definitions ###
      typedef map<string, WINDOW *> windowMapType;
      typedef list<string> commandListType;

      // ### Variables ###
      static const int PRIMARY_OFFSET_HEIGHT;
      static const int PRIMARY_OFFSET_WIDTH;
      static const int SECONDARY_OFFSET_HEIGHT;
      static const int SECONDARY_OFFSET_WIDTH;

      static const int PAD_OFFSET_HEIGHT;
      static const int PAD_OFFSET_WIDTH;
      static const int PROMPT_OFFSET_HEIGHT;
      static const int PROMPT_OFFSET_WIDTH;
      static const int INNER_OFFSET_HEIGHT;
      static const int INNER_OFFSET_WIDTH;
      static const int STATUS_OFFSET_HEIGHT;
      static const int STATUS_OFFSET_WIDTH;

      // ### Members ###
      void print(const string &);
      void keypress(const char);
      const commandListType::size_type dequeue(commandListType &);

    protected:
      StdinCoord *_getCoords(WINDOW *);
      void _createWindows();
      void _moveCursorToPrompt();
      WINDOW *_addWindow(const int, const int, const int, const int);
      void _removeWindow(WINDOW *);
      WINDOW *_addPad(const int, const int);
      void _printStatus();
      void _leftArrow();
      void _rightArrow();
      void _scrollUp();
      void _scrollDown();
      void _enterPressed();
      void _backspacePressed();
      void _logCoord(const string &, StdinCoord *);

    private:
      FIELD *_fields[2];
      FORM *_form;
      WINDOW *_primaryWindow;
      WINDOW *_secondaryWindow;
      WINDOW *_pad;
      StdinCoord *_scrCoord;
      StdinCoord *_primaryCoord;
      StdinCoord *_secondaryCoord;
      StdinCoord *_innerCoord;
      StdinCoord *_padCoord;
      StdinCoord *_promptCoord;
      StdinCoord *_statusCoord;
      commandListType _commandList;
      string _prompt;
      string _input;
      size_t _pos;
      size_t _hpos;
      windowMapType _windowMap;
      int _maxScroll;
      int _index;
  };

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // namespace openaprs
#endif
