#ifndef __MODULE_STOMP_CLASS_STOMP_H
#define __MODULE_STOMP_CLASS_STOMP_H

#include <list>
#include <string>
#include <set>

#include "App_Log.h"
#include "Command.h"

#include <openframe/openframe.h>
#include <openframe/scoped_lock.h>

namespace stomp {
  using std::string;
  using std::list;
  using std::set;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/
  class DBI_Stomp;

  class Stomp : public openframe::OpenFrame_Abstract, public openframe::ThreadMessenger {
    public:
      Stomp();
      virtual ~Stomp();

      // ### Type Definitions ###
      typedef set<pthread_t> threadSetType;

      // ### Public Members ###
      void initializeSystem();
      void initializeThreads();
      void initializeCommands();
      void initializeDatabase();
      void deinitializeSystem();
      void deinitializeThreads();
      void deinitializeCommands();
      void deinitializeDatabase();
      const bool done() {
        bool ret;
        openframe::scoped_lock slock(&_done_l);
        ret = _done;
        return ret;
      } // done
      void done(const bool done) {
        openframe::scoped_lock slock(&_done_l);
        _done = done;
      } // done

      const bool queue(openframe::ThreadMessage *tm) {
        threadSetType::iterator ptr = _stompThreads.begin();
        pthread_t tid = *ptr;
        return ThreadMessenger::queue(tid, tm);
      } // queue

      const bool dequeue(openframe::threadMessages_t &tms) {
        threadSetType::iterator ptr = _stompThreads.begin();
        pthread_t tid = *ptr;
        return openframe::ThreadMessenger::dequeue(tid, tms);
      } // queue

      DBI_Stomp *dbi() { return _dbi; }

      static void *StompThread(void *);

    protected:
    private:
      DBI_Stomp *_dbi;
      openframe::OFLock _done_l;
      bool _done;
      threadSetType _stompThreads;
  }; // OpenTest_Abstract

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

  extern "C" {
    extern Stomp *stomp;
  } // extern "C"
} // namespace stomp

#endif
