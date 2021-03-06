#ifndef __MODULE_STOMP_STOMPCOMMANDS_H
#define __MODULE_STOMP_STOMPCOMMANDS_H

#include "Command.h"

namespace stomp {
  using openaprs::Command;
  using openaprs::CommandHandler;
  using openaprs::Packet;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

CREATE_COMMAND(stdinSMSCDELIVERSM);
CREATE_COMMAND(stdinESMESUBMITSM);

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

}
#endif
