dnl ##########################################################################
dnl ## Dynamic Networking Solutions                                         ##
dnl ##########################################################################
dnl ## OpenAPRS, Internet APRS MySQL Injector                               ##
dnl ## Copyright (C) 1999 Gregory A. Carter                                 ##
dnl ##                    Dynamic Networking Solutions                      ##
dnl ##                                                                      ##
dnl ## This program is free software; you can redistribute it and/or modify ##
dnl ## it under the terms of the GNU General Public License as published by ##
dnl ## the Free Software Foundation; either version 1, or (at your option)  ##
dnl ## any later version.                                                   ##
dnl ##                                                                      ##
dnl ## This program is distributed in the hope that it will be useful,      ##
dnl ## but WITHOUT ANY WARRANTY; without even the implied warranty of       ##
dnl ## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        ##
dnl ## GNU General Public License for more details.                         ##
dnl ##                                                                      ##
dnl ## You should have received a copy of the GNU General Public License    ##
dnl ## along with this program; if not, write to the Free Software          ##
dnl ## Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            ##
dnl ##########################################################################
dnl $Id: configure.in,v 1.1 2005/11/21 18:16:02 omni Exp $
dnl ##########################################################################

dnl #############################################
dnl ## Run config.sub/guess to find out system ##
dnl #############################################

AC_REVISION($Revision: 1.1 $)
AC_INIT

echo "##########################################################################"
echo "## OpenAPRS: Internet APRS MySQL Injector                               ##"
echo "##########################################################################"
echo "## OpenAPRS, Internet APRS MySQL Injector                               ##"
echo "## Copyright (C) 1999 Gregory A. Carter                                 ##"
echo "##                    Dynamic Networking Solutions                      ##"
echo "##                                                                      ##"
echo "## This program is free software; you can redistribute it and/or modify ##"
echo "## it under the terms of the GNU General Public License as published by ##"
echo "## the Free Software Foundation; either version 1, or (at your option)  ##"
echo "## any later version.                                                   ##"
echo "##                                                                      ##"
echo "## This program is distributed in the hope that it will be useful,      ##"
echo "## but WITHOUT ANY WARRANTY; without even the implied warranty of       ##"
echo "## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        ##"
echo "## GNU General Public License for more details.                         ##"
echo "##                                                                      ##"
echo "## You should have received a copy of the GNU General Public License    ##"
echo "## along with this program; if not, write to the Free Software          ##"
echo "## Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            ##"
echo "##########################################################################"

AC_CANONICAL_SYSTEM
AC_CONFIG_SRCDIR([src/main.cpp])
AM_INIT_AUTOMAKE([openaprs], [1.01.09])
AM_CONFIG_HEADER(include/defs.h)

AC_ISC_POSIX

dnl Stops missing crap
AM_MAINTAINER_MODE

dnl Configure libtool
AM_PROG_LIBTOOL

CFLAGS="-Wall -pipe -g"
CXXFLAGS="-Wall -pipe -g"
CXX="g++"

ofincludedir="`pwd`/libraries/openframe/include"
oflibdir="`pwd`/libraries/openframe/lib"

osincludedir="`pwd`/libraries/openstats/include"
oslibdir="`pwd`/libraries/openstats/lib"

stincludedir="`pwd`/libraries/stomp/include"
stlibdir="`pwd`/libraries/stomp/lib"

satincludedir="`pwd`/libraries/opensat/include"
satlibdir="`pwd`/libraries/opensat/lib"

LIBS="-L$oflibdir -L$satlibdir -L$oslibdir -L$stlibdir -lopenframe -lopensat -lopenstats -lstomp"
INCLUDEDIR="-I$ofincludedir -I$satincludedir -I$osincludedir -I$stincludedir"

MYSQL_INCLUDE=""
MYSQL_LIB=""

prefix="`pwd`"
bindir="`pwd`"
libdir="`pwd`/lib"

dnl #######################################
dnl ## Check for standard shell programs ##
dnl #######################################

AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_LANG([C++])

AC_PROG_MAKE_SET(gmake)

dnl #####################################
dnl ## Look for db.h on Linux machines ##
dnl #####################################

AC_MSG_CHECKING(for db.h)
if test -f "/usr/include/db1/db.h"; then
  CXXFLAGS="$CXXFLAGS -I/usr/include/db1"
fi
AC_MSG_RESULT(found)

dnl ##################
dnl ## Define LINUX ##
dnl ##################

dnl OS Check
case "$host_alias" in
*linux*)
  CPPFLAGS="$CPPFLAGS -DLINUX";
   LIBS="$LIBS -ldl -rdynamic";
esac

dnl ----------------------------------------------------------
dnl ----------------------------------------------------------
dnl
dnl System specific checks
dnl
dnl ----------------------------------------------------------
dnl ----------------------------------------------------------

dnl AC_CHECK_LIB(z,deflate, [], AC_MSG_ERROR(libz required))

echo "####################"
echo "## Solaris Checks ##"
echo "####################"

dnl AC_CHECK_LIB(socket,socket)
dnl AC_CHECK_LIB(nsl,gethostbyname)

AC_LIBRARY_NET
AC_CHECK_LIB(resolv, hstrerror)
AC_CHECK_LIB(dl, dlopen)

dnl ----------------------------------------------------------
dnl
dnl Checking for headers, functions, and a type declarations  
dnl 

echo "###################"
echo "## Header Checks ##"
echo "###################"

AC_CHECK_HEADERS(sys/file.h unistd.h sys/stat.h sys/ioctl.h sys/resource.h )
AC_CHECK_HEADERS(sys/time.h asm/ioctls.h xti_inet.h sys/filio.h getopt.h)
AC_CHECK_HEADERS(sys/types.h sys/socket.h netinet/in.h sys/un.h)

AC_HEADER_STDC
AC_HEADER_TIME
AC_TYPE_SIGNAL
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UID_T

dnl AIX keeps in_addr_t in /usr/include/netinet/in.h
dnl AC_MSG_CHECKING([for in_addr_t])
AC_CACHE_VAL(ac_cv_type_$1,
[AC_EGREP_CPP(dnl
changequote(<<,>>)dnl
<<in_addr_t[^a-zA-Z_0-9]>>dnl
changequote([,]), [#include <sys/types.h>
#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif], ac_cv_type_in_addr_t=yes, ac_cv_type_in_addr_t=no)])dnl

dnl AC_MSG_RESULT([$ac_cv_type_in_addr_t])
if test $ac_cv_type_in_addr_t = no; then
  AC_DEFINE(in_addr_t, u_int, [ ])
fi

dnl ############################
dnl ## Check for header files ##
dnl ############################

AC_HEADER_STDC
AC_CHECK_HEADERS(sys/time.h)

dnl #######################################
dnl ## Check for compile specific things ##
dnl #######################################

AC_C_CONST
AC_C_INLINE
AC_HEADER_TIME
AC_STRUCT_TM
AC_STRUCT_TIMEZONE

AC_CHECK_LIB(ssl, SSL_library_init)

dnl ######################
dnl ## Look for mysql.h ##
dnl ######################

echo "##################"
echo "## mySQL Checks ##"
echo "##################"

AC_MSG_CHECKING(for mysql.h)
if test -f "/usr/local/include/mysql/mysql.h"; then
  MYSQL_INCLUDE=/usr/local/include/mysql
elif test -f "/usr/include/mysql/mysql.h"; then
  MYSQL_INCLUDE=/usr/include/mysql
elif test -f "/usr/local/mysql/include/mysql.h"; then
  MYSQL_INCLUDE=/usr/local/mysql/include
elif test -f "/usr/local/include/mysql.h"; then
  MYSQL_INCLUDE=/usr/local/include
elif test -f "/usr/include/mysql.h"; then
  MYSQL_INCLUDE=/usr/include
elif test -f "/usr/local/mysql/include/mysql/mysql.h"; then
  MYSQL_INCLUDE=/usr/local/mysql/include/mysql
else
  AC_MSG_ERROR( Unable to find mySQL)
fi
AC_MSG_RESULT(found)

dnl ################################
dnl ## Look for libmysqlclient.so ##
dnl ################################

dnl AC_MSG_CHECKING(for libmysqlclient)
dnl if test -f "/usr/local/lib/mysql/libmysqlclient.so"; then
dnl  MYSQL_LIB=/usr/local/lib/mysql
dnl elif test -f "/usr/local/lib/libmysqlclient.so"; then
dnl  MYSQL_LIB=/usr/local/lib
dnl elif test -f "/usr/lib/libmysqlclient.so"; then
dnl  MYSQL_LIB=/usr/lib
dnl elif test -f "/usr/lib/mysql/libmysqlclient.so"; then
dnl  MYSQL_LIB=/usr/lib/mysql
dnl elif test -f "/usr/local/lib/mysql/libmysqlclient.a"; then
dnl  MYSQL_LIB=/usr/local/lib/mysql  
dnl elif test -f "/usr/local/mysql/lib/mysql/libmysqlclient.so"; then
dnl  MYSQL_LIB=/usr/local/mysql/lib/mysql
dnl else
dnl  AC_MSG_ERROR( Unable to find mySQL)
dnl fi  
dnl AC_MSG_RESULT(found)

AC_CHECK_HEADER([mysql/mysql.h], [break])
AC_CHECK_LIB([mysqlclient], [main])

dnl #########################
dnl ## Check for memcached ##
dnl #########################

echo "#########################"
echo "## libmemcached Checks ##"
echo "#########################"

PKG_CHECK_MODULES(DEPS, libmemcached >= 0.8.0) AC_SUBST(DEPS_CFLAGS) AC_SUBST(DEPS_LIBS)

echo "#######################"
echo "## libmysql++ Checks ##"
echo "#######################"

CPPFLAGS="$CPPFLAGS -I/usr/include/mysql -I/usr/include/mysql++"
AC_CHECK_HEADER([mysql++/mysql++.h],,
[
  echo "Can't find mysql++ headers."
  exit 1
],[])
AC_CHECK_LIB([mysqlpp], [main])

echo "####################"
echo "## pthread Checks ##"
echo "####################"

AC_CHECK_LIB(pthread, pthread_create)

echo "####################"
echo "## ncurses Checks ##"
echo "####################"

AC_CHECK_LIB(ncurses, newwin)

dnl #################################
dnl ## Check for library functions ##
dnl #################################

AC_CHECK_FUNCS(gettimeofday select)

CXXFLAGS="$CXXFLAGS $INCLUDEDIR -I`pwd`/include -I`pwd`/libraries/putmysql -I`pwd`/libraries/aprs -I$MYSQL_INCLUDE $DEPS_CFLAGS"
LIBS="$LIBS -L`pwd`/lib $DEPS_LIBS"

dnl #########################
dnl ##Create our Makefiles ##
dnl #########################

AC_SUBST(CXXFLAGS)
AC_SUBST(INCLTDL)
AC_SUBST(LIBLTDL)
AC_SUBST(LIBTOOL_DEPS)
AC_SUBST(bindir)

AC_DEFINE_UNQUOTED(SPATH, "`pwd`/openaprs")
AC_DEFINE([SPATH], ["`pwd`/openaprs"])

ALL_OUTPUT_FILES="$OUTPUT_MODULES src/Makefile Makefile"

dnl AC_OUTPUT($ALL_OUTPUT_FILES)

AC_OUTPUT(libraries/Makefile \
          libraries/aprs/Makefile \
          libraries/putmysql/Makefile \
          modules/Makefile \
          modules/apns/Makefile \
          modules/dcc/Makefile \
          modules/mgmt/Makefile \
          modules/rollup/Makefile \
          modules/stomp/Makefile \
          modules/sat/Makefile \
          src/Makefile \
          Makefile)

echo ""
echo "  ##########################################################################"
echo "  ## Configured Successfully                                              ##"
echo "  ##########################################################################"
echo "  ## Step 1:                                                              ##"
echo "  ##                                                                      ##"
echo "  ## Now your OpenAPRS is ready to compile, perform the following unix    ##"
echo "  ## commands to compile and install.                                     ##"
echo "  ##                                                                      ##"
echo "  ## 1) gmake install                                                     ##"
echo "  ##                                                                      ##"
echo "  ## You will need to edit etc/example.conf and save it to the desired    ##"
echo "  ## filename.                                                            ##"
echo "  ##                                                                      ##"
echo "  ##########################################################################"
echo "  ## Step 2:                                                              ##"
echo "  ##                                                                      ##"
echo "  ## Once you're ready to start your OpenAPRS process type the following  ##"
echo "  ## unix commands.                                                       ##"
echo "  ##                                                                      ##"
echo "  ## 1) ./openaprs -d -c etc/<config>.conf                                ##"
echo "  ##                                                                      ##"
echo "  ##########################################################################"
echo ""
echo "  Version: $VERSION"
