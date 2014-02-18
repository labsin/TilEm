# - Try to find TiConv

include(LibFindMacros)

# dependency
libfind_package(TiConv Glib)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(TiConv_PKGCONF ticonv)

find_path(TiConv_INCLUDE_DIR
  NAMES ticonv.h
  PATHS ${TiConv_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(TiConv_LIBRARY
  NAMES ticonv
  PATHS ${TiConv_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(TiConv_PROCESS_INCLUDES TiConv_INCLUDE_DIR Glib_INCLUDE_DIR)
set(TiConv_PROCESS_LIBS TiConv_LIBRARY Glib_LIBRARY)
libfind_process(TiConv)
