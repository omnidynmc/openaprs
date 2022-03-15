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
 $Id: config.h,v 1.1 2005/11/21 18:16:03 omni Exp $
 **************************************************************************/

#ifndef __OPENAPRS_CONFIG_H
#define __OPENAPRS_CONFIG_H

namespace openaprs {

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

#define CHANLEN			200		/* Max channel length */
#define TOPICLEN		160		/* Max topic length */
#define KEYLEN			23		/* Max key length */
#define BANMASKLEN		50		/* a guess? */
#define NICKLEN                 9		/* Max nick length */
#define HOSTLEN                 63		/* Max host length */
#define USERLEN                 10		/* Max user (ident) length */
#define REALLEN			60		/* Max real name length */

#define EMAILLEN		64		/* Max email address length */

#define PASSLEN			60		/* Max pass length */

#define MAXPARA         	15		/* Max parameters from the irc server */
#define MAXMSG          	512		/* Max message length */

#define PATH_PS 		"ps"		/* Path to ps */
#define MAXPSBUF		512		/* Max ps buffer size */

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

}
#endif
