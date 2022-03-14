/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, Internet APRS MySQL Injector                               **
 ** Copyright (C) 1999 Gregory A. Carter                                 **
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
 $Id: openaprs.h,v 1.1 2005/11/21 18:16:03 omni Exp $
 **************************************************************************/

#ifndef __OPENAPRS_H
#define __OPENAPRS_H

#include <map>
#include <string>

#include <math.h>
#include <stdarg.h>

#include "Cfg.h"
#include "PutMySQL.h"

#include "defs.h"
#include "config.h"
#include "openaprs_string.h"

using std::string;

using namespace openaprs;
using namespace putmysql;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/***********
 ** mySQL **
 ***********/
#define OPENAPRS_SQL_HOST CFG_STRING("openaprs.sql.host", "openaprs.sql.host")
#define OPENAPRS_SQL_USER CFG_STRING("openaprs.sql.user", "openaprs.sql.user")
#define OPENAPRS_SQL_PASS CFG_STRING("openaprs.sql.pass", "openaprs.sql.pass")
#define OPENAPRS_SQL_DB CFG_STRING("openaprs.sql.db", "openaprs_sql.db")

#define SQL_HOST OPENAPRS_SQL_HOST
#define SQL_USER OPENAPRS_SQL_USER
#define SQL_PASS OPENAPRS_SQL_PASS
#define SQL_DB OPENAPRS_SQL_DB

#define OPENAPRS_SQL_LOGIN                   OPENAPRS_SQL_HOST, OPENAPRS_SQL_USER, OPENAPRS_SQL_PASS, OPENAPRS_SQL_DB

#define NULL_OPTION(x, y)		((x->getString(y).length() > 0) ? (string("'") + x->getString(y) + string("'")) : (string("NULL"))).c_str()
#define NULL_ESCAPE(x, y)		((x->getString(y).length() > 0) ? (string("'") + Sql->Escape(x->getString(y)) + string("'")) : (string("NULL"))).c_str()
#define NULL_CHAR(x)			((strlen(x) > 0) ? (string("'") + string(x) + string("'")) : string("NULL")).c_str()
#define NULL_STRING(x)			((x.length() > 0) ? (string("'") + x + string("'")) : string("NULL")).c_str()

/************
 ** Server **
 ************/

#define OPENAPRS_CONFIG_COMPRESS			CFG_BOOL("compress", false)
#define OPENAPRS_CONFIG_CONNECT_BURST		CFG_STRING("openaprs.burst", "burst/auth.burst")
#define OPENAPRS_CONFIG_PORT			CFG_STRING("openaprs.server.port", "23")
#define OPENAPRS_CONFIG_CREATE			CFG_STRING("openaprs.create.callsign", "CALLSIGN")
#define OPENAPRS_CONFIG_PREAUTH			CFG_BOOL("openaprs.preauth", false)
#define OPENAPRS_CONFIG_SCRIPTING			CFG_BOOL("openaprs.scripting", true)
#define OPENAPRS_CONFIG_SERVER			CFG_STRING("openaprs.server", "127.0.0.1")
#define OPENAPRS_CONFIG_SERVER_NUMERIC		CFG_STRING("numeric", "ZX")
#define OPENAPRS_CONFIG_SERVER_NAME		CFG_STRING("name", "openaprs.netburst.org")
#define OPENAPRS_CONFIG_VERSION_REPLY		CFG_STRING("version_reply", "Internet Relay Chat Protocol Burster")
#define OPENAPRS_CONFIG_CREATE_DESTINATION	CFG_STRING("openaprs.create.destination", "APOA00")

/************
 ** Timers **
 ************/

#define OPENAPRS_TIMER_CONNECT			60
#define OPENAPRS_TIMER_CREATE			CFG_INT("openaprs.sql.create.interval", 30)
#define OPENAPRS_TIMER_CLEANUP			CFG_INT("openaprs.sql.cleanup.interval", 500)
#define OPENAPRS_TIMER_STATS			CFG_INT("openaprs.stats.report", 3600)
#define OPENAPRS_TIMER_LOG			2

/************************
 ** Logging Facilities **
 ************************/

#define OPENAPRS_LOG_APRS			"openaprs.log.aprs"
#define OPENAPRS_LOG_STDOUT			"openaprs.log.stdout"
#define OPENAPRS_LOG_NORMAL			"openaprs.log.normal"
#define OPENAPRS_LOG_DB_ERROR			"openaprs.log.db.error"
#define OPENAPRS_LOG_VERIFY			"openaprs.log.verify"
#define OPENAPRS_LOG_MONITOR			"openaprs.log.monitor"
#define OPENAPRS_LOG_SQL			"openaprs.log.sql"
#define OPENAPRS_LOG_DECAY			"openaprs.log.decay"
#define OPENAPRS_LOG_DEBUG			"openaprs.log.debug"
#define OPENAPRS_LOG_DEBUG_EVENT		"openaprs.log.debug.event"
#define OPENAPRS_LOG_DEBUG_DROPPED		"openaprs.log.debug.dropped"
#define OPENAPRS_LOG_DEBUG_PACKET			"openaprs.log.debug.packet"
#define OPENAPRS_LOG_DEBUG_SOCKET			"openaprs.log.debug.socket"
#define OPENAPRS_LOG_FLUSH			CFG_BOOL("openaprs.log.flush", false)
#define OPENAPRS_LOG_FILTER			CFG_STRING("openaprs.log.filter", "root")
#define OPENAPRS_LOG_FILTER_EVENT		CFG_STRING("openaprs.log.filter.event", "root")

/*******************
 ** Default Paths **
 *******************/

#define OPENAPRS_PATH_CONFIG		CFG_STRING("openaprs.path.config", "etc/openaprs.conf")
#define OPENAPRS_PATH_PID			CFG_STRING("openaprs.path.pid", "openaprs.pid")
#define OPENAPRS_PATH_LOGS		CFG_STRING("openaprs.path.logs", "logs/")
#define OPENAPRS_PATH_LOGS_OPENAPRS		CFG_STRING("openaprs.path.logs.openaprs", "openaprs.log")
#define OPENAPRS_PATH_LOGS_APRS			CFG_STRING("openaprs.path.logs.aprs", "openaprs.aprs.log")
#define OPENAPRS_PATH_LOGS_VERIFY		CFG_STRING("openaprs.path.logs.verify", "openaprs.verify.log")
#define OPENAPRS_PATH_LOGS_MONITOR		CFG_STRING("openaprs.path.logs.monitor", "openaprs.monitor.log")
#define OPENAPRS_PATH_LOGS_SQL			CFG_STRING("openaprs.path.logs.sql", "openaprs.sql.log")
#define OPENAPRS_PATH_LOGS_DECAY		CFG_STRING("openaprs.path.logs.decay", "openaprs.decay.log")
#define OPENAPRS_PATH_LOGS_DEBUG		CFG_STRING("openaprs.path.logs.debug", "openaprs.debug.log")
#define OPENAPRS_PATH_LOGS_DEBUG_EVENT	CFG_STRING("openaprs.path.logs.debug.event", "openaprs.debug.event.log")
#define OPENAPRS_PATH_LOGS_DEBUG_DROPPED	CFG_STRING("openaprs.path.logs.debug.dropped", "openaprs.debug.dropped.log")
#define OPENAPRS_PATH_LOGS_DEBUG_PACKET	CFG_STRING("openaprs.path.logs.debug.packet", "openaprs.debug.packet.log")
#define OPENAPRS_PATH_LOGS_DEBUG_SOCKET	CFG_STRING("openaprs.path.logs.debug.socket", "openaprs.debug.socket.log")

#define TRUE 				1
#define FALSE				0

#define OPENAPRS_MIN(a, b)			((a) < (b) ? (a) : (b))
#define OPENAPRS_MAX(a, b)			((a) > (b) ? (a) : (b))

// time frames
#define OPENAPRS_TIME_YEAR			31536000
#define OPENAPRS_TIME_MONTH			1814400
#define OPENAPRS_TIME_WEEK			604800
#define OPENAPRS_TIME_DAY			86400
#define OPENAPRS_TIME_HOUR			3600
#define OPENAPRS_TIME_MINUTE			60

#define DEFAULT_QUIT			"Shutdown"
#define DEFAULT_RESTART			"Restart"

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

#define ERR_BUFFER_ALLOC	"buffer allocation error"

#define ERROR_REPLY_BUFFER_ALLOCATION	"Buffer allocation error."
#define ERROR_REPLY_FAILED_JUMP		"Failed to jump."
#define ERROR_REPLY_FAILED_RESTART	"Failed to restart."

#define MAXBUF			(1024*1024)

#define UNUSED(x)		(void) x

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

int is_dir(const char *);
int send_auth(const char *, const char *, const char *);
int sectotf(time_t, struct timeframe *);

void deactivate(const char *);
void header();
void usage();
void version();

extern PutMySQL *Sql;

#endif
