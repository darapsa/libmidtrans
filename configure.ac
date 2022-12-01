AC_INIT([midtrans], [0.0], [prabowo@darapsa.org])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AM_PROG_AR
LT_INIT
PKG_CHECK_MODULES([DEPS], [libcurl json-c])
AC_CANONICAL_HOST
case $host in
	*arm*apple-darwin1* ) ios=true;;
	*) ios=false;;
esac
AM_CONDITIONAL([IOS], [test "x$ios" = xtrue])
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_TYPE_SIZE_T
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile midtrans.pc])
AC_CONFIG_MACRO_DIRS([m4])
AC_OUTPUT
