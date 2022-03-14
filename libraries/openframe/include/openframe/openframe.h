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

#ifndef LIBOPENFRAME_OPENFRAME_H
#define LIBOPENFRAME_OPENFRAME_H

#include "Vars.h"
#include "ByteData.h"
#include "ConfController.h"
#include "ConnectionManager.h"
#ifdef FEAT_MYSQL
#include "DBI.h"
#endif
#include "Fifo.h"
#include "Intval.h"
#include "LineBuffer.h"
#include "ListenController.h"
#include "Logger.h"
#include "logging/loglevel.h"
#include "md5.h"
#include "md5wrapper.h"
#include "noCaseCompare.h"
#include "OFLock.h"
#include "OpenFrame_Abstract.h"
#include "openframe.h"
#include "OpenFrame_Log.h"
#include "PeerController.h"
#include "Refcount.h"
#include "Result.h"
#include "Serialize.h"
#include "scoped_lock.h"
#include "SocketBase.h"
#include "Stopwatch.h"
#include "stringify.h"
#include "StringToken.h"
#include "StringTool.h"
#include "TestController.h"
#include "ThreadMessage.h"
#include "Url.h"
#include "VarController.h"
#include "Vars.h"

namespace openframe {

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/


} // namespace openframe

#endif
