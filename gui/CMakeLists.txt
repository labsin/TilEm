if(NOT DEFINED GUI_TARGET)
    set(GUI_TARGET tilem2)
endif()

project(${GUI_TARGET})
set(SRC
    tilem2.c
    debugger.c
    disasmview.c
    memview.c
    memmodel.c
    breakpoints.c
    address.c
    keypaddlg.c
    memory.c
    emulator.c
    emucore.c
    emuwin.c
    event.c
    preferences.c
    linksetup.c
    audiosetup.c
    skinops.c
    tool.c
    config.c
    link.c
    macro.c
    gifencod.c
    animatedgif.c
    screenshot.c
    pbar.c
    animation.c
    audiodev.c
    files.c
    icons.c
    keybindings.c
    menu.c
    rcvmenu.c
    sendfile.c
    filedlg.c
    fixedtreeview.c
    ti81prg.c
    charmap.c)

add_definitions( -DHAVE_CONFIG_H )
include_directories(${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Emu_SOURCE_DIR}
    ${Db_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${GTK2_INCLUDE_DIRS}
    ${TiCalcs2_INCLUDE_DIR})
if(${HAVE_LIBSDL})
    include_directories(${SDL_INCLUDE_DIR})
endif()

add_executable(${GUI_TARGET}
    ${SRC}
)

set(LIBS
    Emu Db
    ${Glib_LIBRARIES} ${GTK2_LIBRARIES}
    ${TiConv_LIBRARIES} ${TiCalcs2_LIBRARIES})

if(${HAVE_LIBSDL})
    set(LIBS ${LIBS}
        ${SDL_LIBRARY})
endif()   
message(LIBS: ${LIBS})

target_link_libraries(${GUI_TARGET}
    ${LIBS}) 

install (TARGETS ${GUI_TARGET} RUNTIME DESTINATION bin)
file (GLOB HEADER_FILES *.h)
install (FILES ${HEADER_FILES} DESTINATION include)
