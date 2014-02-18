cmake_minimum_required(VERSION 2.8)
set(PACKAGE_NAME TilEm)
set(PROJECT_EMAIL tilem-devel@lists.sourceforge.net)
set(PACKAGE_URL http://tilem.sourceforge.net/)
set(PACKAGE_TARNAME tilem)
set(PACKAGE_VERSION_MAJOR 2)
set(PACKAGE_VERSION_MINOR 0)
set(PACKAGE_VERSION_PATCH 0)
set(PACKAGE_VERSION ${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_PATCH})
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "https://bugs.launchpad.net/ubuntu/+filebug")

set(CPACK_PACKAGE_NAME ${PACKAGE_NAME})
set(CPACK_PACKAGE_VERSION ${PACKAGE_VERSION})
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.14), libgdk-pixbuf2.0-0 (>= 2.22.0), libglib2.0-0 (>= 2.37.3), libgtk2.0-0 (>= 2.18.0), libticables2-6 (>= 1.3.4), libticalcs2-11 (>= 1.1.8), libticonv7 (>= 1.1.4), libtifiles2-9 (>= 1.1.6)")
set(CPACK_PACKAGE_CONTACT ${PROJECT_EMAIL})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GTK+ TI Z80 calculator emulator
 TilEm is an emulator and debugger for Texas Instruments' Z80-based graphing
 calculators. It can emulate any of the following calculator models:
  * TI-73 / TI-73 Explorer
  * TI-76.fr
  * TI-81
  * TI-82
  * TI-82 STATS / TI-82 STATS.fr
  * TI-83
  * TI-83 Plus / TI-83 Plus Silver Edition / TI-83 Plus.fr
  * TI-84 Plus / TI-84 Plus Silver Edition / TI-84 pocket.fr
  * TI-85
  * TI-86
 TilEm fully supports all known versions of the above calculators (as of 2012),
 and attempts to reproduce the behavior of the original calculator hardware as
 faithfully as possible. In addition, TilEm can emulate the TI-Nspire's virtual
 TI-84 Plus mode. This is currently experimental, and some programs may not work
 correctly.
 .
 TilEm runs on the X Window System on GNU/Linux and other Unix-like platforms,
 as well as on Microsoft Windows, and any other platform supported by the GTK+
 library. In addition to the emulation, TilEm 2 provide a lot of extra features,
 such as:
  * Fully featured debugger
  * Grabbing screenshots and recording gif (animations)
  * Virtual linking (through libticables)
  * Flash writing and erasing
  * Application and OS loading
  * Scripting using macros")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CPACK_DEBIAN_PACKAGE_SECTION "universe/math")
set(CPACK_DEBIAN_PACKAGE_SUGGESTS "libsdl1.2debian (>= 1.2.0-1)")

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
SET(CPACK_PACKAGE_VENDOR "${PACKAGE_TARNAME}")
SET(CPACK_PACKAGE_VERSION_MAJOR "${PACKAGE_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${PACKAGE_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${PACKAGE_VERSION_PATCH}")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${PACKAGE_TARNAME} ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${PACKAGE_TARNAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
SET(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
set(CPACK_GENERATOR DEB)

INCLUDE(CPack)

# List executables

find_package(Glib 2.12.0 REQUIRED)
find_package(GTK2 2.6 REQUIRED)
find_package(TiCalcs2 REQUIRED)
find_package(TiConv REQUIRED)
find_package(SDL)

if(SDL_FOUND)
    set(HAVE_LIBSDL true)
    if(NOT DEFINED ENABLE_AUDIO)
        set(ENABLE_AUDIO true)
    endif()
endif()

INCLUDE (CheckIncludeFiles)
INCLUDE (CheckFunctionExists)
include(CheckLibraryExists)
include (TestBigEndian)
TEST_BIG_ENDIAN(BIG_ENDIAN)
set(ENABLE_NLS)
CHECK_FUNCTION_EXISTS (bind_textdomain_codeset HAVE_BIND_TEXTDOMAIN_CODESET)
CHECK_FUNCTION_EXISTS (lround HAVE_LROUND)
CHECK_INCLUDE_FILES (stdint.h HAVE_STDINT_H)
CHECK_LIBRARY_EXISTS (TiCables ticables_cable_set_raw "" HAVE_TICABLES_CABLE_SET_RAW)
CHECK_FUNCTION_EXISTS (__sync_synchronize HAVE___SYNC_SYNCHRONIZE)
IF(NOT HAVE_UINTPTR_T)
  IF("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
    SET(uintptr_t "uint64_t")
  ELSE()
    SET(uintptr_t "uint32_t")
  ENDIF()
ENDIF(NOT HAVE_UINTPTR_T)

add_subdirectory(emu)
add_subdirectory(db)
add_subdirectory(data)
add_subdirectory(gui)

configure_file(config.cmake.h.in config.h)
