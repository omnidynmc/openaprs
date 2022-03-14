#ifndef __LIBOPENFRAME_COUNTER_H
#define __LIBOPENFRAME_COUNTER_H

#include <iostream>

namespace openframe {

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  template <typename T>
  struct counter {
      static long objects_created;
      static long int objects_alive;
      counter() {
        ++objects_created;
        ++objects_alive;
      } // counter

    protected:
      ~counter() {
        --objects_alive;
      } // ~counter
  }; // counter

  template<typename T> long int counter<T>::objects_created(0);
  template<typename T> long int counter<T>::objects_alive(0);

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // namespace openframe
#endif
