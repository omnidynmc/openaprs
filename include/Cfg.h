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
 $Id: misc.h,v 1.1 2005/11/21 18:16:03 omni Exp $
 **************************************************************************/

#ifndef __OPENAPRS_CONFIGMACRO_H
#define __OPENAPRS_CONFIGMACRO_H

namespace openaprs {

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

#define APP_CFG_BOOL(y, z) (app->cfg->get_bool(y, z))
#define APP_CFG_DOUBLE(y, z) (app->cfg->get_double(y, z))
#define APP_CFG_INT(y, z) (app->cfg->get_int(y, z))
#define APP_CFG_STRING(y, z) (app->cfg->get_string(y, z))
#define APP_CFG_UINT(y, z) (app->cfg->get_uint(y, z))

#define CFG_BOOL(y, z) APP_CFG_BOOL(y, z)
#define CFG_DOUBLE(y, z) APP_CFG_DOUBLE(y, z)
#define CFG_INT(y, z) APP_CFG_INT(y, z)
#define CFG_STRING(y, z) APP_CFG_STRING(y, z)
#define CFG_UINT(y, z) APP_CFG_UINT(y, z)

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

}
#endif
