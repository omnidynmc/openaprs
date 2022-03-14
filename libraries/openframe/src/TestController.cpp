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

#include <openframe/StringToken.h>
#include <openframe/StringTool.h>
#include <openframe/TestController.h>
#include <openframe/stringify.h>

namespace openframe {
  /**************************************************************************
   ** TestController Class                                                 **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  const std::string TestController::DEFAULT_TITLE	= "TEST";
  const size_t TestController::DEFAULT_PAD		= 70;

  TestController::TestController(const testSizeType numTests) : _numTests(numTests), _numTestsFailed(0), _numTestsRun(0), _title(DEFAULT_TITLE) {
    _timeTotal.Start();
    _timeNext.Start();
  } // TestController::TestController

  TestController::TestController(const testSizeType numTests, const std::string &title) : _numTests(numTests), _numTestsFailed(0), _numTestsRun(0), _title(title) {
    _timeTotal.Start();
    _timeNext.Start();
  } // TestController::TestController

  TestController::~TestController() {
  } // TestController::~TestController

  void TestController::begin() {
    begin(_title);
  } // TestController::begin

  void TestController::begin(const std::string &title) {
    std::cout << std::endl << StringTool::ipad("### BEGIN: "+ title +" ", "#", DEFAULT_PAD) << std::endl;
  } // TestController::begin

  void TestController::end() {
    std::cout << StringTool::ipad("### SUMMARY ", "#", DEFAULT_PAD) << std::endl
         << "### " << std::setfill(' ') << std::setw(10) << _numTestsRun << " tests run" << std::endl
         << "### " << std::setfill(' ') << std::setw(10) << _numTests << " tests expected " << std::endl
         << "### " << std::setfill(' ') << std::setw(10) << _numTestsFailed << " tests failed" << std::endl;

    if (_numTestsRun != _numTests && _numTests)
      std::cout << "!!! WARNING: TEST COUNT MISMATCH" << std::endl;

    if (_numTestsFailed)
      std::cout << "!!! WARNING: FAILURES DETECTED" << std::endl;

    std::cout << std::setprecision(5) << std::fixed << "### Total time " << _timeTotal.Time() << " seconds" << std::endl
         << StringTool::ipad("### END: "+_title+" - "+string(_numTestsFailed ? "FAIL " : "PASS "), "#", DEFAULT_PAD) << std::endl << std::endl;
  } // TestController::end

  void TestController::comment(const std::string &mesg) {
    std::cout << "### " << mesg << std::endl;
  } // TestController::comment

  void TestController::title(const std::string &mesg) {
    std::cout << std::endl << std::setw(DEFAULT_PAD) << std::setfill('#') << "" << std::endl;
    std::cout << "### " << mesg.substr(0, DEFAULT_PAD-4) << std::endl;
    std::cout << std::setw(DEFAULT_PAD) << std::setfill('#') << "" << std::endl;
  } // TestController::title

  const bool TestController::ok(const bool isOK, const std::string &mesg) {
    std::stringstream out;

    Lock();

    _numTestsRun++;
    if (!isOK)
      _numTestsFailed++;

    out << (isOK ? " ok " : " not ok ")
        << _numTestsRun << " - "
        << mesg
        << " (" << std::setprecision(5) << std::fixed << _timeNext.Time() << " seconds)";

    std::cout << out.str() << std::endl;

    _timeNext.Start();

    Unlock();

    return isOK;
  } // TestController::ok

  const bool TestController::like(const std::string &str, const std::string &reg, const std::string &mesg) {
    StringTool::regexMatchListType rl;
    Lock();
    bool isOK = StringTool::ereg(reg, str, rl);
    Unlock();
    return ok(isOK, mesg);
  } // TestController::like

  const bool TestController::unlike(const std::string &str, const std::string &reg, const std::string &mesg) {
    StringTool::regexMatchListType rl;
    Lock();
    bool isOK = !StringTool::ereg(reg, str, rl);
    Unlock();
    return ok(isOK, mesg);
  } // TestController::unlike

  const bool TestController::is(const int m1, const int m2, const std::string &mesg) {
    bool isOK = (m1 == m2);
    return ok(isOK, mesg);
  } // TestController::is

  const bool TestController::notok(const bool isOK, const std::string &mesg) {
    return ok(!isOK, mesg);
  } // TestController::not

  const bool TestController::greater(const int m1, const int m2, const std::string &mesg) {
    bool isOK = (m1 > m2);
    return ok(isOK, mesg);
  } // TestController::is

  const bool TestController::less(const int m1, const int m2, const std::string &mesg) {
    bool isOK = (m1 < m2);
    return ok(isOK, mesg);
  } // TestController::is

  const bool TestController::match(const std::string &m1, const std::string &m2, const std::string &mesg) {
    bool isOK = (m1 == m2);
    return ok(isOK, mesg);
  } // TestController::match

  const bool TestController::notmatch(const std::string &m1, const std::string &m2, const std::string &mesg) {
    bool isOK = (m1 != m2);
    return ok(isOK, mesg);
  } // TestController::notmatch

  const bool TestController::isnt(const int m1, const int m2, const std::string &mesg) {
    bool isOK = (m1 != m2);
    return ok(isOK, mesg);
  } // TestController::isnt

  void TestController::sleepfor(time_t interval) {
    TestController::comment("sleeping for "+stringify<time_t>(interval)+" second"+(interval == 1 ? "" : "s") );
    while(interval > 0) {
      sleep(1);
      interval--;
      if (interval && (interval % 10) == 0)
        TestController::comment("sleep: "+stringify<time_t>(interval)+" second"+(interval == 1 ? "" : "s")+" remaining before continuing");
    } // while
  } // sleepfor

  const int TestController::run(const std::string &command, const std::string &write_str, const time_t waitsec) {
    FILE *file;

//    Lock();
    std::cout << "### Running: " << command << std::endl;

    file = popen(command.c_str(), "w");
    if (file == NULL) {
      std::cout << "### Run Failed: " << command << std::endl;
      return 1;
    } // if

    if ( write_str.length() ) {
      int ret = fwrite(write_str.data(), write_str.length(), 1, file);
      if (ret < 1) std::cout << "### Write to RUN Failed " << std::endl;
    } // if

    if (waitsec) TestController::sleepfor(waitsec);

    int exitno = pclose(file);
    std::cout << "### Exit Status: " << exitno << std::endl;
//    Unlock();

    return exitno;
  } // TestController::run

} // namespace openframe
