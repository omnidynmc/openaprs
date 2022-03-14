#ifndef __OPENFRAME_STDINCOMMANDS_H
#define __OPENFRAME_STDINCOMMANDS_H

#include <map>
#include <string>
#include <list>

#include <time.h>
#include <sys/time.h>

#include "Command.h"

namespace openframe {
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

CREATE_COMMAND(stdinADDMODULE);
CREATE_COMMAND(stdinBINARY);
CREATE_COMMAND(stdinBOOKMARKS);
CREATE_COMMAND(stdinCAT);
CREATE_COMMAND(stdinDTREE);
CREATE_COMMAND(stdinHELP);
CREATE_COMMAND(stdinHISTORY);
CREATE_COMMAND(stdinQUIT);
CREATE_COMMAND(stdinSETCONFIG);
CREATE_COMMAND(stdinRELOADMODULE);
CREATE_COMMAND(stdinREMOVEMODULE);
CREATE_COMMAND(stdinSHOWCONFIG);

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // namespace openframe
#endif
