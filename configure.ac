AC_INIT([midtrans], [0.0], [prabowo@darapsa.org])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AM_PROG_AR
LT_INIT
PKG_CHECK_MODULES([DEPS], [libcurl json-c])
AC_CANONICAL_HOST
case $host in
	*arm*apple-darwin1* ) ios=true;;
	*) ios=false;;
esac
AM_CONDITIONAL([IOS], [test "x$ios" = xtrue])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
