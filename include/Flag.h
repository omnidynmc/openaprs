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
 $Id: Flag.h,v 1.1 2005/11/21 18:16:03 omni Exp $
 **************************************************************************/

#ifndef __OPENAPRS_FLAG_H
#define __OPENAPRS_FLAG_H

namespace openaprs {

using std::string;
using std::stringstream;
using std::ofstream;
using std::ios;
using std::ios_base;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class Flag {
  public:
    /**********************
     ** Type Definitions **
     **********************/

    typedef unsigned int flagType;

    /******************************
     ** Constructor / Destructor **
     ******************************/

    Flag()
      { myFlags = 0; }

    virtual ~Flag() {}

    /******************
     ** Flag Members **
     ******************/

    const virtual bool addFlag(const flagType addMe) {
      if (myFlags & addMe)
        return false;
      
      myFlags |= addMe;
      
      return true;
    } // addFlag

    inline virtual const bool removeFlag(const flagType removeMe) {
      if (!(myFlags & removeMe))
        return false;
    
      myFlags &= ~removeMe;
    
      return true;
    } // removeFlag

    inline virtual const bool findFlag(const flagType findMe) const {
      return myFlags & findMe ? true : false;
    } // findFlag

    const flagType getFlags() const
      { return myFlags; }

  protected:
  private:
    /***************
     ** Variables **
     ***************/

    flagType myFlags;				// Variable to store my flags.
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // namespace openaprs
#endif
