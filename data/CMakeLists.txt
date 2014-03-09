if(NOT DEFINED DATA_TARGET)
    set(DATA_TARGET tilem-data)
endif()
configure_file(desktop/tilem2.desktop.cmake desktop/tilem2.desktop)

project(${DATA_TARGET})

install(FILES keybindings.ini DESTINATION ${DATA_INSTALL_DIR})
install(DIRECTORY symbols skins DESTINATION ${DATA_INSTALL_DIR})

foreach(_PKGICONDIR
        hicolor/16x16/actions/
        hicolor/16x16/status/
        hicolor/24x24/actions/)
    install(DIRECTORY icons/${_PKGICONDIR} DESTINATION ${DATA_INSTALL_DIR}/${_PKGICONDIR})
endforeach()

foreach(_SHAREDICONDIR
        hicolor/16x16/apps/
        hicolor/22x22/apps/
        hicolor/24x24/apps/
        hicolor/32x32/apps/
        hicolor/48x48/apps/)
    install(DIRECTORY icons/${_SHAREDICONDIR} DESTINATION ${ICON_INSTALL_DIR}/${_SHAREDICONDIR})
endforeach()

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/desktop/tilem2.desktop DESTINATION ${XDG_APPS_DIR})
install(FILES desktop/tilem2.xml DESTINATION ${MIME_INSTALL_DIR})
install(FILES icons/hicolor/index.theme DESTINATION ${ICON_INSTALL_DIR}/hicolor)
