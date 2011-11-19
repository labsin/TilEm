AC_PREREQ(2.67)
AC_INIT([TilEm], [pre-release], [tilem-devel@lists.sourceforge.net],
  [tilem], [http://tilem.sourceforge.net/])
AC_CONFIG_SRCDIR([emu/tilem.h])

# Checks for programs

AC_PROG_CC
AC_PROG_CPP
AC_ARG_VAR(OPT_CFLAGS,
  [Additional C compiler flags used for optimizing critical areas of
  the code (default: -O3 if using GCC)])
if test "x$GCC" = "xyes" ; then
  CFLAGS="$CFLAGS -W -Wall -Wwrite-strings"
  if test "x$OPT_CFLAGS" = "x" ; then
    OPT_CFLAGS="-O3"
  fi
fi

AC_CHECK_TOOL(AR, [ar], [false])
AC_ARG_VAR(AR, [Static library archiver])
AC_ARG_VAR(AR_FLAGS, [Flags to pass to ar to build a static library])
if test "x$AR_FLAGS" = "x" ; then
  AR_FLAGS=cru
fi

AC_PROG_RANLIB
AC_ARG_VAR(RANLIB, [Program to make a static library linkable])

AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries

m4_define(with_flags, [
  save_cflags="$CFLAGS"
  save_libs="$LIBS"
  CFLAGS="$CFLAGS $$1_CFLAGS"
  LIBS="$LIBS $$1_LIBS"
  $2
  CFLAGS="$save_cflags"
  LIBS="$save_libs"
])

# GLib and GTK+

PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.6.0
                       glib-2.0 >= 2.12.0
                       gthread-2.0)

AC_ARG_ENABLE([gtk-deprecated],
  AS_HELP_STRING([--disable-gtk-deprecated], [Disable deprecated GTK+ API]),
  [ enable_gtk_deprecated=$enableval ], [ enable_gtk_deprecated=yes ])
if test "x$enable_gtk_deprecated" = "xno" ; then
  GTK_CFLAGS="$GTK_CFLAGS -DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGSEAL_ENABLE"
fi

# If using the native Windows version of GTK+, be sure to use
# -mms-bitfields for all compilation.  Also, use -mwindows for linking
# GUI programs.

# (If not using pkg-config, you're on your own)

if test "x$PKG_CONFIG" != "x" ; then
  gtk_target=`$PKG_CONFIG --variable=target gtk+-2.0`
fi

if test "x$gtk_target" = "xwin32" && test "x$GCC" = "xyes" ; then
  CFLAGS="$CFLAGS -mms-bitfields"
  GUI_LDFLAGS="-mwindows"
  LIBS="-lcomdlg32 -lshell32 -lole32 $LIBS"
else
  GUI_LDFLAGS=""
fi

AC_SUBST(GUI_LDFLAGS)

with_flags(GTK,
  [ AC_CHECK_FUNC(gtk_init, [ have_gtk=yes ], [ have_gtk=no ]) ])
if test "x$have_gtk" != "xyes" ; then
  AC_MSG_ERROR([GTK+ 2.x libraries not found or not usable.
You must install a recent version of GTK+ 2.x, including the
-dev/-devel packages if appropriate.])
fi

# Libticalcs2 and related libraries

AC_ARG_WITH(ticalcs2,
  AS_HELP_STRING([--with-ticalcs2], [use libticalcs2 (default)]),
    [ use_ticalcs=$withval ], [ use_ticalcs=auto ])
AC_ARG_WITH(calcprotocols,
  AS_HELP_STRING([--with-calcprotocols], [use libcalcprotocols]),
    [ use_calcprotocols=$withval ], [ use_calcprotocols=auto ])

init_function=ticalcs_library_init

if test "x$TICALCS_LIBS" = "x" &&
   test "x$use_ticalcs" != "xno" &&
   test "x$use_calcprotocols" != "xyes" ; then
  PKG_CHECK_MODULES(TICALCS, ticalcs2 ticables2 tifiles2 ticonv, [ : ], [ : ])
fi

if test "x$TICALCS_LIBS" = "x" &&
   test "x$use_ticalcs" != "xyes" &&
   test "x$use_calcprotocols" != "xno" ; then
  PKG_CHECK_MODULES(CALCPROTOCOLS, calcprotocols calccables calcfiles calcconv,
    [ TICALCS_CFLAGS="$CALCPROTOCOLS_CFLAGS -I\$(top_srcdir)/tilibs-compat"
      TICALCS_LIBS="$CALCPROTOCOLS_LIBS"
      init_function=calcprotocols_library_init ],
    [ : ])
fi

with_flags(TICALCS,
  [ AC_CHECK_FUNC($init_function, [ have_ticalcs=yes ], [ have_ticalcs=no ]) ])
if test "x$have_ticalcs" != "xyes" ; then
  AC_MSG_ERROR([libticalcs2/libcalcprotocols not found or not usable.
You must install one of these two libraries, including the
-dev/-devel packages if appropriate.])
fi

# Checks for header files

AC_HEADER_STDC

# Checks for system and compiler characteristics

AC_C_BIGENDIAN
AC_C_INLINE
AC_C_RESTRICT
AC_TYPE_UINTPTR_T

# Output

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile
                 emu/Makefile
                 db/Makefile
                 gui/Makefile])
AC_OUTPUT
