# - Try to find TiCables 2.0

include(LibFindMacros)

# dependency
libfind_package(TiCables2 Glib)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(TiCables2_PKGCONF ticables2)

find_path(TiCables2_INCLUDE_DIR
  NAMES ticables.h
  PATHS ${TiCables2_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(TiCables2_LIBRARY
  NAMES ticables2
  PATHS ${TiCables2_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(TiCables2_PROCESS_INCLUDE TiCables2_INCLUDE_DIR Glib_INCLUDE_DIR)
set(TiCables2_PROCESS_LIB TiCables2_LIBRARY Glib_LIBRARY})
libfind_process(TiCables2)
