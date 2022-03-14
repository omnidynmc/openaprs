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

#ifndef __LIBOPENFRAME_FIFO_H
#define __LIBOPENFRAME_FIFO_H

#include <string>
#include <vector>

namespace openframe {
/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  typedef std::vector<char> fifo_t;
  typedef fifo_t::iterator fifo_itr;
  typedef fifo_t::const_iterator fifo_citr;
  typedef fifo_t::size_type fifo_st;

  class Fifo : protected std::vector<char> {
    public:
      Fifo(const fifo_st res_arg=1000000);
      ~Fifo();

      Fifo &add_charbuf(const char *buf, const size_t len);
      void clear();
      bool next_line(std::string &ret);
      bool next_cstring(std::string &ret);
      bool next_delim(std::string &ret, const char c);
      bool next_bytes(const fifo_st len, std::string &ret);
      void erase_to_pos();
      fifo_st length() const { return size(); }

    private:
      fifo_itr _pos;
  }; // class Fifo

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/
} // namespace openframe
#endif
