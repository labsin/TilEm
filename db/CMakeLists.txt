if(NOT DEFINED DB_TARGET)
    set(DB_TARGET Db)
endif()
project(${DB_TARGET})

set(SRC
    disasm.c
    prevaddr.c
    listing.c
    lstfile.c)

add_definitions( -DHAVE_CONFIG_H )
include_directories( ${CMAKE_BINARY_DIR}
    ${Emu_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR})

add_library(${DB_TARGET} STATIC
    ${SRC})
