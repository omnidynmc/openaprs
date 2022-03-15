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
 $Id: Server.cpp,v 1.9 2006/03/21 20:55:12 omni Exp $
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
#include <vector>

#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <ncurses.h>

#include "OpenAPRS_Abstract.h"
#include "StdinController.h"
#include "StringTool.h"

#include "openaprs.h"

namespace openaprs {
  using std::string;
  using std::list;
  using std::ofstream;
  using std::ostream;
  using std::stringstream;
  using std::ios;
  using std::endl;
  using std::cout;

/**************************************************************************
 ** Server Class                                                         **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

  StdinController::StdinController(const string &prompt, const int maxScroll) : _prompt(prompt), _maxScroll(maxScroll) {
    _primaryWindow = NULL;
    _pad = NULL;
    _index = 0;
    _pos = 0;
    _hpos = 0;
    _input = "";

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    refresh();

    _createWindows();
  } // StdinController::StdinController

  StdinController::~StdinController() {
    _removeWindow(_primaryWindow);
    _removeWindow(_pad);

    delete _scrCoord;
    delete _innerCoord;
    delete _promptCoord;
    delete _statusCoord;

    endwin();
  } // StdinController::~StdinController

/***************
 ** Variables **
 ***************/

  const int StdinController::PRIMARY_OFFSET_HEIGHT		= 4;
  const int StdinController::PRIMARY_OFFSET_WIDTH		= 0;
  const int StdinController::SECONDARY_OFFSET_HEIGHT		= 4;
  const int StdinController::SECONDARY_OFFSET_WIDTH		= 0;

  const int StdinController::INNER_OFFSET_HEIGHT		= 2;
  const int StdinController::INNER_OFFSET_WIDTH			= 2;
  const int StdinController::PROMPT_OFFSET_HEIGHT		= 2;
  const int StdinController::PROMPT_OFFSET_WIDTH		= 0;
  const int StdinController::STATUS_OFFSET_HEIGHT		= 3;
  const int StdinController::STATUS_OFFSET_WIDTH		= 0;

  // ### Public Members ###
  void StdinController::_createWindows() {
    // first we need our screen sizes
    _scrCoord = _getCoords(stdscr);

    // create main window with border
    _primaryWindow = _addWindow(_scrCoord->height()-PRIMARY_OFFSET_HEIGHT, _scrCoord->width()-PRIMARY_OFFSET_WIDTH, 0, 0);
    _primaryCoord = _getCoords(_primaryWindow);

    _secondaryWindow = _addWindow(4, _scrCoord->width()-SECONDARY_OFFSET_WIDTH, _scrCoord->height()-SECONDARY_OFFSET_HEIGHT, 0);
    _secondaryCoord = _getCoords(_secondaryWindow);

    // create with default borders
    box(_primaryWindow, 0, 0);
    box(_secondaryWindow, 0, 0);
    wrefresh(_primaryWindow);
    wrefresh(_secondaryWindow);

    _innerCoord = new StdinCoord(_primaryCoord->height()-INNER_OFFSET_HEIGHT, _primaryCoord->width()-INNER_OFFSET_WIDTH, 1, 1);
    _padCoord = new StdinCoord(_innerCoord->height()+_maxScroll, _innerCoord->width(), 1, 1);
    _promptCoord = new StdinCoord(1, _prompt.length()+1, _secondaryCoord->height()-PROMPT_OFFSET_HEIGHT, 1);
    _statusCoord = new StdinCoord(1, _secondaryCoord->width()-3, _secondaryCoord->height()-STATUS_OFFSET_HEIGHT, 1);

//_logCoord("statusCoord", _statusCoord);
//_logCoord("innerCoord", _innerCoord);
//_logCoord("padCoord", _padCoord);
//_logCoord("promptCoord", _promptCoord);

    // create pad
    _pad = _addPad(_padCoord->height(), _padCoord->width());
    scrollok(_pad, 1);

//    _printStatus();
    _hpos = _prompt.length();
    mvwprintw(_secondaryWindow, _promptCoord->y(), _promptCoord->x(), "%s", _prompt.c_str());
    wmove(_secondaryWindow, _promptCoord->y(), _promptCoord->width());
    wrefresh(_secondaryWindow);
//    _printStatus();
  } // StdinController::_createWindows

  const StdinController::commandListType::size_type StdinController::dequeue(commandListType &commandList) {
    commandList = _commandList;
    _commandList.clear();
    return commandList.size();
  } // StdinController::dequeue

  void StdinController::_logCoord(const string &label, StdinCoord *coord) {
    _logf("Coord %s: height %d, width %d, y %d, x %d", label.c_str(), coord->height(),
          coord->width(), coord->y(), coord->x());
  } // _logCoord

  void StdinController::_moveCursorToPrompt() {
    int y, x;

    getyx(_secondaryWindow, y, x);
    wmove(_secondaryWindow, y, x);
    wrefresh(_secondaryWindow);
  } // StdinController::_moveCursorToPrompt

  StdinCoord *StdinController::_getCoords(WINDOW *window) {
    StdinCoord *ret;
    int y, x;
    int max_y, max_x;

    assert(window != NULL);

    getyx(window, y, x);
    getmaxyx(window, max_y, max_x);

    ret = new StdinCoord(max_y, max_x, y, x);

    return ret;
  } // StdinController::_getCoords

  WINDOW *StdinController::_addWindow(const int height, const int width, const int y, const int x) {
    WINDOW *ret;

    // create new window
    ret = newwin(height, width, y, x);

    notimeout(ret, true);
    nodelay(ret, true);
    keypad(ret, true);

    return ret;
  } // StdinController::_addWindow

  void StdinController::_removeWindow(WINDOW *window) {
    if (window == NULL)
      return;

    // remove any borders displayed
    //wborder(window, ' ', ' ', ' ',' ',' ',' ',' ',' ');
    //wrefresh(window);

    delwin(window);
  } // StdinController::_removeWindow

  WINDOW *StdinController::_addPad(const int height, const int width) {
    WINDOW *ret;

    // create new window
    ret = newpad(height, width);

    return ret;
  } // StdinController::_addPad

  void StdinController::_printStatus() {
    stringstream s;
    string out;
    int y, x, pad_y, pad_x;

    getyx(_secondaryWindow, y, x);
    getyx(_pad, pad_y, pad_x);

    s.str("");
    s << "Line "
      << pad_y
      << ", index "
      << _index << "/" << (pad_y-_innerCoord->height() > 0 ? pad_y-_innerCoord->height() : 0)
      << ", cursor "
      << _pos
      << "/"
      << _input.length();
    out = s.str();

    openframe::StringTool::pad(out, " ", _statusCoord->width());
    mvwprintw(_secondaryWindow, _statusCoord->y(), _statusCoord->x(), "%s", out.c_str());
    wmove(_secondaryWindow, y, x);
  } // StdinController::_printStatus

  void StdinController::_scrollUp() {
    int pad_y, pad_x;

    if (_index == 0)
      return;

    getyx(_pad, pad_y, pad_x);
    prefresh(_pad, --_index, 0, 1, 1, _innerCoord->height(), _innerCoord->width());

    _printStatus();
  } // StdinController::_scrollUp

  void StdinController::_scrollDown() {
    int y, x, pad_y, pad_x;

    getyx(stdscr, y, x);
    getyx(_pad, pad_y, pad_x);

    if (_index >= pad_y-_innerCoord->height() || pad_y-_innerCoord->height() < 0)
      return;

    prefresh(_pad, ++_index, 0, 1, 1, _innerCoord->height(), _innerCoord->width());

    _printStatus();
  } // StdinController::_scrollDown

  void StdinController::print(const string &out) {
    int pad_y, pad_x;

    if (!out.length())
      return;

    wprintw(_pad, "%s", out.c_str());

    getyx(_pad, pad_y, pad_x);
    if (pad_y > _innerCoord->height())
      _index = pad_y - _innerCoord->height();

    prefresh(_pad, _index+1, 0, 1, 1, _innerCoord->height(), _innerCoord->width());

    _printStatus();
  } // StdinController::print

  void StdinController::_enterPressed() {
    int pad_y, pad_x;

    if (!_input.length())
      return;

    //_input.append(1, '\n');

    //wprintw(_pad, "%s", _input.c_str());

    //getyx(_pad, pad_y, pad_x);
    //if (pad_y > _innerCoord->height())
    //  _index = pad_y - _innerCoord->height();
    _commandList.push_back(_input);

    wmove(_secondaryWindow, _promptCoord->y(), _promptCoord->width());
    for(int i=0; i < _input.length(); i++)
      waddch(_secondaryWindow, ' ');
    wmove(_secondaryWindow, _promptCoord->y(), _promptCoord->width());
    _input = "";
    _pos = 0;
     //_printStatus();
  } // StdinController::_enterPressed

  void StdinController::_backspacePressed() {
    if (!_input.length())
      return;

    _pos--;
    _input.erase(_pos, 1);
    mvwdelch(_secondaryWindow, _promptCoord->y(), _promptCoord->width()+_pos);
  } // StdinController::_backspacePressed

  void StdinController::_leftArrow() {
    if (_pos == _input.length())
      return;

    _pos++;
    wmove(_secondaryWindow, _promptCoord->y(), _promptCoord->width()+_pos);
  } // StdinController::_leftArrow()

  void StdinController::_rightArrow() {
    if (_pos == 0)
      return;

    _pos--;
    wmove(_secondaryWindow, _promptCoord->y(), _promptCoord->width()+_pos);
  } // StdinController::_rightArrow()

  void StdinController::keypress(const char ch1) {
    char ch;

    while((ch = wgetch(_secondaryWindow)) != ERR) {
      switch((unsigned int) ch) {
        case '\x04':		// right arrow
          _rightArrow();
          break;
        case '\x05':		// left arrow
          _leftArrow();
          break;
        case KEY_UP:
        case '\x03':
          _scrollUp();
          break;
        case KEY_DOWN:
        case '\x02':
          _scrollDown();
          break;
        case KEY_BACKSPACE:
        case '\x07':
          _backspacePressed();
          break;
        case KEY_RESIZE:
          mvprintw(_statusCoord->y(), _statusCoord->x(), "Resize detected          ");
          break;
        case KEY_ENTER:
        case '\r':
        case '\n':
          _enterPressed();
          break;
        default:
//_logf("BLAH(%s) (%s)", aprs::StringTool::char2hex(ch).c_str(), aprs::StringTool::char2hex(KEY_ENTER).c_str());
          _input.insert(_pos, 1, ch);

//          if (movingRight())
          mvwinsch(_secondaryWindow, _promptCoord->y(), _promptCoord->width()+_pos++, ch);
          wmove(_secondaryWindow, _promptCoord->y(), _promptCoord->width()+_pos);
          wrefresh(_secondaryWindow);
          _hpos++;
      } // switch
      _printStatus();
    } // while

  } // StdinController::keypress
} // namespace openaprs
