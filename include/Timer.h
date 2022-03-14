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
 $Id: Timer.h,v 1.1 2005/11/21 18:16:03 omni Exp $
 **************************************************************************/

#ifndef __OPENAPRS_TIMER_H
#define __OPENAPRS_TIMER_H

#include <string>

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

class TimerHandler;

class Timer {
  public:
    /**********************
     ** Type Definitions **
     **********************/

    typedef unsigned int timerId;

    /******************************
     ** Constructor / Destructor **
     ******************************/

    Timer(const timerId addId, 
          const time_t addInterval, 
          const unsigned int addRuncount, 
          const bool addPersistent, 
          const bool addDelete, 
          TimerHandler *addTimerHandler) {
      myId = addId;
      myInterval = addInterval;
      myPersistent = addPersistent;
      myRemove = false;
      myDelete = addDelete;
      myRuncount = addRuncount;
      myTimerHandler = addTimerHandler;

      myNextRun = (time(NULL) + addInterval);
    } // Timer

    virtual ~Timer() { }

    /********************
     ** Remove Members **
     ********************/

    inline const bool getDelete() const
      { return myDelete; }

    inline void setDelete(const bool setMe)
      { myDelete = setMe; }

    /****************
     ** Id Members **
     ****************/

    inline const timerId getId() const
      { return myId; }

    inline void setId(const timerId setMe)
      { myId = setMe; }

    /**********************
     ** Interval Members **
     **********************/

    inline const time_t getInterval() const
      { return myInterval; }

    inline void setInterval(const time_t setMe)
      { myInterval = setMe; }

    /******************
     ** Name Members **
     ******************/

    inline const string &getName() const
      { return myName; }

    inline void setName(const string &setMe)
      { myName = setMe; }

    /**********************
     ** Next Run Members **
     **********************/

    inline const time_t getNextRun() const
      { return myNextRun; }

    inline void setNextRun(const time_t setMe)
      { myNextRun = setMe; }

    /************************
     ** Persistent Members **
     ************************/

    inline const bool getPersistent() const
      { return myPersistent; }

    inline void setPersistent(const bool setMe)
      { myPersistent = setMe; }

    /********************
     ** Remove Members **
     ********************/

    inline const bool getRemove() const
      { return myRemove; }

    inline void setRemove(const bool setMe)
      { myRemove = setMe; }

    /***********************
     ** Run Count Members **
     ***********************/

    inline const unsigned int getRuncount() const
      { return myRuncount; }

    inline void setRuncount(const unsigned int setMe)
      { myRuncount = setMe; }

    /**************************
     ** TimerHandler Members **
     **************************/

    inline TimerHandler *getTimerHandler() const
      { return myTimerHandler; }

    inline void setTimerHandler(TimerHandler *setMe)
      { myTimerHandler = setMe; }

  protected:
    TimerHandler *myTimerHandler;			// TimerHandler we reference.
    bool myDelete;					// Should I be deleted (freed)?
    bool myPersistent;					// Should I ever been removed?
    bool myRemove;					// Should I be removed?
    string myName;					// My name.
    time_t myNextRun;					// When should I be run next?
    time_t myInterval;					// How often am I run?
    timerId myId;					// Timer Id I'm assigned to.
    unsigned int myRuncount;				// How many times should I run?

  private:
};

class TimerHandler {
  public:   
    TimerHandler() {}			// Default constructor.
    virtual ~TimerHandler() {}		// Default destructor.
     
    /**********************
     ** Type Definitions **
     **********************/

    typedef Timer::timerId timerId;

    /*********************
     ** OnTimer Members **
     *********************/

    virtual const int OnTimer(Timer *) = 0;
 
    /**********************
     ** Operator Members **
     **********************/
    
  protected:
  private:
    /***************
     ** Variables **
     ***************/
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

// define most common timer ``runcount''
#define TIMER_CONTINUOUS	0
#define TIMER_RUNONCE		1

#define CREATE_TIMER(name) \
class name : public TimerHandler { \
  public: \
    name() {} \
    virtual ~name() {} \
    typedef Timer::timerId timerId; \
    const int OnTimer(Timer *); \
};

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

}
#endif
