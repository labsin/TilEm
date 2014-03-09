cmake_minimum_required(VERSION 2.8)

set(APPLICATION_NAME tilem)
set(APPLICATION_NAME_LOWER tilem)
set(APPLICAION_URL "http://tilem.sourceforge.net/")
set(APPLICAION_BUGREPORT "https://bugs.launchpad.net/ubuntu/+filebug") 
set(APPLICAION_TARNAME tilem)
set(APPLICATION_VERSION_MAJOR 2)
set(APPLICATION_VERSION_MINOR 0)
set(APPLICATION_VERSION_PATCH 0)
set(APPLICATION_VERSION ${APPLICATION_VERSION_MAJOR}.${APPLICATION_VERSION_MINOR}.${APPLICATION_VERSION_PATCH})
set(APPLICATION_STRING ${APPLICATION_TARNAME}-${APPLICATION_VERSION})
set(APPLICAION_EMAIL "Tilem Maillist <tilem-devel@lists.sourceforge.net>")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# add definitions
include(DefineCMakeDefaults)
include(DefinePlatformDefaults)
include(DefineCompilerFlags)
include(DefineInstallationPaths)
include(DefineOptions.cmake)
include(CPackConfig.cmake)

# disallow in-source build
include(MacroEnsureOutOfSourceBuild)
macro_ensure_out_of_source_build("${PROJECT_NAME} requires an out of source build. Please create a separate build directory and run 'cmake /path/to/${PROJECT_NAME} [options]' there.")

# list executables
find_package(Glib 2.12.0 REQUIRED)
find_package(GTK2 2.6 REQUIRED)
find_package(TiCalcs2 REQUIRED)
find_package(TiConv REQUIRED)

if(WITH_SDL)
    find_package(SDL)
    set(HAVE_LIBSDL true)

    if(SDL_FOUND)
        if(NOT DEFINED ENABLE_AUDIO)
            set(ENABLE_AUDIO true)
        endif()
    endif()
endif()

include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckLibraryExists)

include(TestBigEndian)
TEST_BIG_ENDIAN(BIG_ENDIAN)

CHECK_FUNCTION_EXISTS(bind_textdomain_codeset HAVE_BIND_TEXTDOMAIN_CODESET)
CHECK_FUNCTION_EXISTS(lround HAVE_LROUND)
CHECK_INCLUDE_FILES(stdint.h HAVE_STDINT_H)
CHECK_LIBRARY_EXISTS(TiCables ticables_cable_set_raw "" HAVE_TICABLES_CABLE_SET_RAW)
CHECK_FUNCTION_EXISTS(__sync_synchronize HAVE___SYNC_SYNCHRONIZE)

if(NOT HAVE_UINTPTR_T)
  if("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
    set(uintptr_t "uint64_t")
  else()
    set(uintptr_t "uint32_t")
  endif()
endif(NOT HAVE_UINTPTR_T)

# configure file
configure_file(config.h.cmake config.h)

# find translatable files
file(GLOB_RECURSE I18N_SRC_FILES
     RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
     **.c)
list(SORT I18N_SRC_FILES)

# add subdirectories
add_subdirectory(emu)
add_subdirectory(db)
add_subdirectory(data)
add_subdirectory(gui)
add_subdirectory(doc)
add_subdirectory(po)

# report
message(STATUS "********************************************")
message(STATUS "********** ${PROJECT_NAME} build options : **********")

message(STATUS "SDL support: ${WITH_SDL}")
message(STATUS "NLS support: ${ENABLE_NLS}")
message(STATUS "********************************************")
