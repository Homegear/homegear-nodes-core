#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(homegar-nodes-core, m4_esyscmd_s([/bin/sh getVersion.sh]), sathya@laufers.net)
AC_CONFIG_AUX_DIR(cfg)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([variable/variable-in/VariableIn.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_PREFIX_DEFAULT(/usr)

dnl AM_MAINTAINER_MODE

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL

# Libraries
LT_INIT

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_C_CONST
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_CHECK_FUNCS([floor memchr memset pow select socket strchr strerror strstr strtol])

AC_CANONICAL_HOST
case $host_os in
	darwin* )
		CPPFLAGS="$CPPFLAGS -DMACOSSYSTEM"
		;;
	linux*)
		CPPFLAGS="$CPPFLAGS -DLINUXSYSTEM"
		;;
	*BSD*)
		CPPFLAGS="$CPPFLAGS -DBSDSYSTEM -D_GLIBCXX_USE_C99 -D_GLIBCXX_USE_C99_MATH -D_GLIBCXX_USE_C99_MATH_TR1 -D_WITH_DPRINTF"
		;;
	*)
		AC_MSG_ERROR([Your platform is not currently supported])
	        ;;
esac

#AC_ARG_ENABLE(debug, AS_HELP_STRING([--enable-debug], [enable debugging, default: no]), [case "${enableval}" in yes) debug=true ;; no)  debug=false ;; *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;; esac], [debug=false])
#AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")

AC_OUTPUT(Makefile average/Makefile basic-logic/Makefile catch/Makefile comment/Makefile debug/Makefile exec/Makefile function/Makefile gpio/Makefile homegear-event/Makefile http/Makefile influxdb/Makefile javascript/Makefile light/Makefile link/Makefile modbus/Makefile mqtt/Makefile notification/Makefile parsers/Makefile passthrough/Makefile ping/Makefile presence-light/Makefile press-pattern/Makefile pulsecounter/Makefile python/Makefile python-wrapper/Makefile rbe/Makefile serial/Makefile synchronous/Makefile status/Makefile storage/Makefile tcp/Makefile template/Makefile timers/Makefile tls-config/Makefile tls-server-config/Makefile udp/Makefile unit-test-helper/Makefile variable/Makefile)
