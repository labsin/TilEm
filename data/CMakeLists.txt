if(NOT DEFINED DATA_TARGET)
    set(DATA_TARGET tilem-data)
endif()
if(NOT DEFINED DATA_DIR)
    set(DATA_DIR share/tilem2)
endif()

project(${DATA_TARGET})
install(FILES keybindings.ini DESTINATION ${DATA_DIR})
install(DIRECTORY symbols skins DESTINATION ${DATA_DIR})
foreach(_PKGICONDIR
        hicolor/16x16/actions/
        hicolor/16x16/status/
        hicolor/24x24/actions/)
    install(DIRECTORY icons/${_PKGICONDIR} DESTINATION ${DATA_DIR}/${_PKGICONDIR})
endforeach()
foreach(_SHAREDICONDIR
        hicolor/16x16/apps/
        hicolor/22x22/apps/
        hicolor/24x24/apps/
        hicolor/32x32/apps/
        hicolor/48x48/apps/)
    install(DIRECTORY icons/${_SHAREDICONDIR} DESTINATION share/icons/${_SHAREDICONDIR})
endforeach()
install(FILES desktop/tilem2.desktop DESTINATION share/applications)
install(FILES desktop/tilem2.xml DESTINATION share/mime)
install(FILES icons/hicolor/index.theme DESTINATION ${DATA_DIR}/icons/hicolor)
