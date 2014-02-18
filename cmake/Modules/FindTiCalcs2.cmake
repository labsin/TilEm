# - Try to find TiCalcs2

include(LibFindMacros)

# Dependencies
libfind_package(TiCalcs2 TiCables2)
libfind_package(TiCalcs2 TiFiles2)
libfind_package(TiCalcs2 Glib)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(TiCalcs2_PKGCONF ticalcs2)

find_path(TiCalcs2_INCLUDE_DIR
  NAMES ticalcs.h
  PATHS ${TiCalcs2_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(TiCalcs2_LIBRARY
  NAMES ticalcs2
  PATHS ${TiCalcs2_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(TiCalcs2_PROCESS_INCLUDES TiCalcs2_INCLUDE_DIR TiCables2_INCLUDE_DIR TiFiles2_INCLUDE_DIR Glib_INCLUDE_DIR)
set(TiCalcs2_PROCESS_LIBS TiCalcs2_LIBRARY TiCables2_LIBRARY TiFiles2_LIBRARY Glib_LIBRARY)
libfind_process(TiCalcs2)
