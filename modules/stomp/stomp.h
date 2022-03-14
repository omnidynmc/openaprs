#ifndef MODULE_STOMP_H
#define MODULE_STOMP_H

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

#define MODULE_STOMP_SQL_HOST		app->cfg->get_string("module.stomp.sql.host", "")
#define MODULE_STOMP_SQL_DB		app->cfg->get_string("module.stomp.sql.db", "")
#define MODULE_STOMP_SQL_USER		app->cfg->get_string("module.stomp.sql.user", "")
#define MODULE_STOMP_SQL_PASS		app->cfg->get_string("module.stomp.sql.pass", "")

#define MODULE_STOMP_HOST		app->cfg->get_string("module.stomp.host", "")
#define MODULE_STOMP_PORT		app->cfg->get_int("module.stomp.port", 61613)
#define MODULE_STOMP_MAX			app->cfg->get_int("module.stomp.max", 1024)

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

#endif
