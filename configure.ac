#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([brasd], [0.1.0], [kitten@seu.edu.cn])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall foreign])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LN_S
AC_CHECK_PROG([XL2TPD], [xl2tpd], [xl2tpd])

if test "$XL2TPD" != "xl2tpd" ; then
    AC_MSG_FAILURE([could not find x2tlpd])
fi

# Checks for libraries.
AC_CHECK_LIB([event], [event_init], [], [AC_MSG_FAILURE([could not find libevent])])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h netdb.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_CHECK_FUNCS([alarm bzero dup2 mkdir socket])

dnl ================================================================
dnl bras-client
dnl ================================================================

AC_ARG_ENABLE([client],
	AS_HELP_STRING([--disable-client],[Do not compile bras-client (default: enabled)]),
	[enable_client=$enableval],
	[enable_client=yes])

if test "x$enable_client" = "xyes" ; then
    PKG_CHECK_MODULES([GTKMM], [gtkmm-2.4], have_client=yes, have_client=no)
    CLIENT_DIR=bras-client
else
    CLIENT_DIR=
fi
AC_SUBST([CLIENT_DIR])

AC_CONFIG_FILES([Makefile src/Makefile data/Makefile bras-client/Makefile bras-client/ui/Makefile debian/Makefile])
AC_OUTPUT
