project(tilem2)
include(FindGettext)
find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext)

set(DOMAIN ${APPLICATION_NAME_LOWER})
set(POT_FILE ${DOMAIN}.pot)
file(GLOB PO_FILES *.po)

add_custom_target(${POT_FILE}
                  COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE} -o ${POT_FILE}
                          -D ${CMAKE_SOURCE_DIR}
                          --from-code=UTF-8
                          --keyword=_
                          --package-name='${PROJECT_NAME}'
                          ${I18N_SRC_FILES})

foreach(PO_FILE ${PO_FILES})
    get_filename_component(LANG ${PO_FILE} NAME_WE)
    gettext_process_po_files(${LANG} ALL PO_FILES ${PO_FILE})
    set(INSTALL_DIR ${LOCALE_INSTALL_DIR}/${LANG}/LC_MESSAGES)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${LANG}.gmo
            DESTINATION ${INSTALL_DIR}
            RENAME ${DOMAIN}.mo)
endforeach(PO_FILE)
