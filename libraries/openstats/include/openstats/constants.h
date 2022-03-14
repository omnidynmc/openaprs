#ifndef __OPENSTATS_CONSTANTS_H
#define __OPENSTATS_CONSTANTS_H

#include <string>
#include <iostream>

namespace openstats {

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  enum returnEnum {
    returnOk 			= 0,
    returnErr 			= -1,
  }; // returnEnum

  enum graphTypeEnum {
    graphTypeCounter			= 0,
    graphTypeGauge			= 1
  }; // graphTypeEnum

  enum useTypeEnum {
    useTypeSum			= 0,
    useTypeMean			= 1,
    useTypeRate			= 2
  }; // useTypeEnum

  enum dataTypeEnum {
    dataTypeInt			= 0,
    dataTypeFloat		= 1,
  }; // dataTypeEnum

  typedef unsigned int statint_t;
  typedef float statfloat_t;

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/
} // namespace openstats
#endif
