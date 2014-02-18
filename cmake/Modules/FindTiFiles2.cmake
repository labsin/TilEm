# - Try to find TiFiles 2.0

include(LibFindMacros)

# dependency
libfind_package(TiFiles2 TiConv)
libfind_package(TiFiles2 Glib)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(TiFiles2_PKGCONF tifiles2)

find_path(TiFiles2_INCLUDE_DIR
  NAMES tifiles.h
  PATHS ${TiFiles2_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(TiFiles2_LIBRARY
  NAMES tifiles2
  PATHS ${TiFiles2_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(TiFiles2_PROCESS_INCLUDE TiFiles2_INCLUDE_DIR TiConv_INCLUDE_DIR Glib_INCLUDE_DIR)
set(TiFiles2_PROCESS_LIB TiFiles2_LIBRARY TiConv_LIBRARY Glib_LIBRARY)
libfind_process(TiFiles2)
