#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([opensat], [1.0.0.1], [gcarter@openaprs.net])
AC_CONFIG_SRCDIR([src/coord.cpp])
AC_CONFIG_HEADERS([include/opensat/config.h])
AM_INIT_AUTOMAKE([opensat], [1.0.0.1])
LT_INIT
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_LIBTOOL

AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_LANG([C++])

prefix="`pwd`"
bindir="`pwd`"
libdir="`pwd`/lib"
includedir="`pwd`/include"

CPPFLAGS="$CPPFLAGS -I/usr/include"
CXXFLAGS="-Wall -pipe -g -I$includedir"
CXX="g++"
LIBS="$LIBS"

# Checks for libraries.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_SUBST(bindir)
AC_SUBST(LIBTOOL_DEPS)

dnl CXXFLAGS="$CXXFLAGS -I`pwd`/include $DEPS_CFLAGS"
LIBS="$LIBS -L`pwd`/lib $DEPS_LIBS"

AC_CONFIG_FILES([Makefile
                 src/Makefile
               ])
AC_OUTPUT
