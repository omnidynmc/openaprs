#ifndef __LIBOPENFRAME_REFCOUNT_H
#define __LIBOPENFRAME_REFCOUNT_H

#include <string>
#include <map>
#include <set>
#include <cassert>

#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "openframe/OFLock.h"

namespace openframe {
  using std::string;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class Refcount {
    public:
      Refcount() : _refcount(1), _dont_delete(false) { }
      virtual ~Refcount() { }

      void retain() {
        _refcount_l.Lock();
        _refcount++;
        _refcount_l.Unlock();
      } // retain
      const unsigned int refcount() {
        _refcount_l.Lock();
        unsigned int ret = _refcount;
        _refcount_l.Unlock();
        return ret;
      } // refcount

      void dont_delete() { _dont_delete = true; }
      void release() {
        _refcount_l.Lock();
        if (--_refcount < 0) assert(false); // bug
        int ref = _refcount;
        _refcount_l.Unlock();

        if (ref == 0 && !_dont_delete) {
          // I'm sure this is really WTF territory but I don't see anywhere
          // that you can't do this to implement a form of reference counting
          // other than accidently deleteing a subclass.  This is total shit
          // but kinda kewl shit.
          delete this;
        } // if
      } // destroy

    private:
      OFLock _refcount_l;
      unsigned int _refcount;
      bool _dont_delete;
  };

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

}
#endif
