AC_INIT([midtrans], [0.0], [prabowo@darapsa.org])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AM_PROG_AR
LT_INIT
PKG_CHECK_MODULES([JSON], [json-c])
AC_CANONICAL_HOST
case $host in
	*arm*apple-darwin1* ) ios=true;;
	*) ios=false;;
esac
AM_CONDITIONAL([IOS], [test "x$ios" = xtrue])
case $host_cpu in
	*wasm* ) wasm=true;;
	*) wasm=false;;
esac
AM_CONDITIONAL([WASM], [test "x$wasm" = xtrue])
AS_IF([test "x$wasm" != xtrue],
      [PKG_CHECK_MODULES([CURL], [libcurl])])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
