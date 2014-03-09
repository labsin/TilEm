# For help take a look at:
# http://www.cmake.org/Wiki/CMake:CPackConfiguration

### general settings
set(CPACK_PACKAGE_NAME ${APPLICATION_NAME})
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README")
set(CPACK_PACKAGE_VENDOR "Tilem Team")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Graphing calculator emulator")

### versions
set(CPACK_PACKAGE_VERSION_MAJOR ${APPLICATION_VERSION_MAJOR}) 
set(CPACK_PACKAGE_VERSION_MINOR ${APPLICATION_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${APPLICATION_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT ${APPLICAION_EMAIL})


### source generator
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES "~$;[.]swp$;/[.]svn/;/[.]git/;.gitignore;/build/;/obj/;tags;cscope.*")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")

if(WIN32)
    set(CPACK_GENERATOR "ZIP")

    ### nsis generator
    find_package(NSIS)
    if(NSIS_MAKE)
        set(CPACK_GENERATOR "${CPACK_GENERATOR};NSIS")
        set(CPACK_NSIS_DISPLAY_NAME "Tilem Texas Instruments emulator")
        set(CPACK_NSIS_COMPRESSOR "/SOLID zlib")
        set(CPACK_NSIS_MENU_LINKS "http://tilem.sourceforge.net/" "tilem homepage")
    endif(NSIS_MAKE)
endif(WIN32)

set(CPACK_PACKAGE_FILE_NAME ${APPLICATION_NAME}-${CPACK_PACKAGE_VERSION})

### DEB
if(UNIX)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.14), libgdk-pixbuf2.0-0 (>= 2.22.0), libglib2.0-0 (>= 2.37.3), libgtk2.0-0 (>= 2.18.0), libticables2-6 (>= 1.3.4), libticalcs2-11 (>= 1.1.8), libticonv7 (>= 1.1.4), libtifiles2-9 (>= 1.1.6)")
set(CPACK_DEBIAN_PACKAGE_SUGGESTS "libsdl1.2debian (>= 1.2.0-1)")
set(CPACK_DEBIAN_PACKAGE_SECTION "universe/math")
set(CPACK_PACKAGE_DESCRIPTION "GTK+ TI Z80 calculator emulator
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
set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
endif()

include(CPack)
