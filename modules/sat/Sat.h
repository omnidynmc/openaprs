#ifndef MODULE_STOMP_CLASS_STOMP_H
#define MODULE_STOMP_CLASS_STOMP_H

#include <list>
#include <string>
#include <set>

#include "App_Log.h"
#include "OpenAPRS_Abstract.h"

#include "Command.h"

#include <openframe/openframe.h>

namespace sat {
  using std::string;
  using std::list;
  using std::set;
  using openaprs::OpenAPRS_Abstract;
  using openframe::OFLock;
  using openframe::ThreadMessenger;
  using openframe::ThreadMessage;
  using openframe::threadMessages_t;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/
  class DBI_Sat;

  class Sat : public OpenAPRS_Abstract, public ThreadMessenger {
    public:
      Sat();
      virtual ~Sat();

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
        _done_l.Lock();
        ret = _done;
        _done_l.Unlock();
        return ret;
      } // done
      void done(const bool done) {
        _done_l.Lock();
        _done = done;
        _done_l.Unlock();
      } // done

      const bool queue(ThreadMessage *tm) {
        threadSetType::iterator ptr = _satThreads.begin();
        pthread_t tid = *ptr;
        return ThreadMessenger::queue(tid, tm);
      } // queue

      const bool dequeue(threadMessages_t &tms) {
        threadSetType::iterator ptr = _satThreads.begin();
        pthread_t tid = *ptr;
        return ThreadMessenger::dequeue(tid, tms);
      } // queue

      DBI_Sat *dbi() { return _dbi; }

      static void *SatThread(void *);

    protected:
    private:
      DBI_Sat *_dbi;
      OFLock _done_l;
      bool _done;
      threadSetType _satThreads;
  }; // OpenTest_Abstract

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

  extern "C" {
    extern Sat *sat;
  } // extern "C"
} // namespace sat

#endif
