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

#include <string>
#include <cstdio>
#include <unistd.h>

#include <openframe/RunTool.h>

namespace openframe {
  /**************************************************************************
   ** RunTool Class                                                        **
   **************************************************************************/

  RunTool::RunTool() {
  } // RunTool::RunTool

  RunTool::~RunTool() {
  } // RunTool::RunTool

  int RunTool::run(const std::string &command, const std::string &write_str, const time_t waitsec) {
    FILE *file;

//    std::cout << "### Running: " << command << std::endl;

    file = popen(command.c_str(), "w");
    if (file == NULL) {
//      std::cout << "### Run Failed: " << command << std::endl;
      return 1;
    } // if

    if ( write_str.length() ) {
      int ret = fwrite(write_str.data(), write_str.length(), 1, file);
//      if (ret < 1) std::cout << "### Write to RUN Failed " << std::endl;
    } // if

    if (waitsec) sleep(waitsec);

    int exitno = pclose(file);
//    std::cout << "### Exit Status: " << exitno << std::endl;

    return exitno;
  } // RunTool::run
} // namespace openframe
