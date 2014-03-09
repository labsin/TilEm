/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
#cmakedefine AC_APPLE_UNIVERSAL_BUILD 1

/* Define to 1 to enable audio support. */
#cmakedefine ENABLE_AUDIO 1

/* always defined to indicate that i18n is enabled */
#cmakedefine ENABLE_NLS 1

/* Define to 1 if you have the `bind_textdomain_codeset' function. */
#cmakedefine HAVE_BIND_TEXTDOMAIN_CODESET 1

/* Define to 1 if you have the `dcgettext' function. */
#cmakedefine HAVE_DCGETTEXT 1

/* Define if the GNU gettext() function is already present or preinstalled. */
#cmakedefine HAVE_GETTEXT 1

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H 1

/* Define if your <locale.h> file defines LC_MESSAGES. */
#cmakedefine HAVE_LC_MESSAGES 1

/* Define to 1 if you have the SDL library. */
#cmakedefine HAVE_LIBSDL 1

/* Define to 1 if you have the <locale.h> header file. */
#cmakedefine HAVE_LOCALE_H 1

/* Define to 1 if you have the `lround' function. */
#cmakedefine HAVE_LROUND 1

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the `ticables_cable_set_raw' function. */
#cmakedefine HAVE_TICABLES_CABLE_SET_RAW 1

/* Define to 1 if the system has the type `uintptr_t'. */
#cmakedefine HAVE_UINTPTR_T 1

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H 1

/* Define to 1 if you have the the `__sync_synchronize' function. */
#cmakedefine HAVE___SYNC_SYNCHRONIZE 1

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "@APPLICATION_BUGREPORT@"

/* Define to the full name of this package. */
#define PACKAGE_NAME "@APPLICATION_NAME@"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "@APPLICATION_STRING@"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "@APPLICATION_TARNAME@"

/* Define to the home page for this package. */
#define PACKAGE_URL "@APPLICATION_URL@"

/* Define to the version of this package. */
#define PACKAGE_VERSION "@APPLICATION_VERSION@"

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS 1

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#cmakedefine BIGENDIAN 1

#if defined BIG_ENDIAN
# define WORDS_BIGENDIAN 1
#endif

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#undef inline
#endif

/* Define to the equivalent of the C99 'restrict' keyword, or to
   nothing if this is not supported.  Do not define if restrict is
   supported directly.  */
#ifdef __restrict__
# define restrict __restrict__
#elif defined __restrict
# define restrict __restrict
#elif defined _Restrict
# define restrict _Restrict
#else
# define restrict
#endif

/* Work around a bug in Sun C++: it does not support _Restrict or
   __restrict__, even though the corresponding Sun C compiler ends up with
   "#define restrict _Restrict" or "#define restrict __restrict__" in the
   previous line.  Perhaps some future version of Sun C++ will work with
   restrict; if so, hopefully it defines __RESTRICT like Sun C does.  */
#if defined __SUNPRO_CC && !defined __RESTRICT
# define _Restrict
# define __restrict__
#endif

/* Define to the type of an unsigned integer type wide enough to hold a
   pointer, if such a type exists, and if the system does not define it. */
#ifndef uintptr_t
# define uintptr_t @uintptr_t@
#endif
