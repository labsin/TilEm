if(NOT DEFINED EMU_TARGET)
    set(EMU_TARGET Emu)
endif()

project(${EMU_TARGET})

include_directories(${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR})

# MAIN
set(CALCS calcs)
set(CALCS_SRCS calcs.c)
set(Z80 z80)
set(Z80_SRCS z80.c)
set(STATE state)
set(STATE_SRCS state.c)
set(ROM rom)
set(ROM_SRCS rom.c)
set(FLASH flash)
set(FLASH_SRCS flash.c)
set(LINK link)
set(LINK_SRCS link.c)
set(KEYPAD keypad)
set(KEYPAD_SRCS keypad.c)
set(LCD lcd)
set(LCD_SRCS lcd.c)
set(CERT cert)
set(CERT_SRCS cert.c)
set(MD5 md5)
set(MD5_SRCS md5.c)
set(TIMERS timers)
set(TIMERS_SRCS timers.c)
set(MONOLCD monolcd)
set(MONOLCD_SRCS monolcd.c)
set(GRAYLCD graylcd)
set(GRAYLCD_SRCS graylcd.c)
set(GRAYIMAGE grayimage)
set(GRAYIMAGE_SRCS grayimage.c)
set(GRAYCOLOR graycolor)
set(GRAYCOLOR_SRCS graycolor.c)
set(AUDIO audio)
set(AUDIO_SRCS audio.c)

#TI-cores
set(X7 x7)
set(X7_SRCS x7/x7_init.c x7/x7_io.c x7/x7_memory.c x7/x7_subcore.c)
set(X1 x1)
set(X1_SRCS x1/x1_init.c x1/x1_io.c x1/x1_memory.c x1/x1_subcore.c)
set(X2 x2)
set(X2_SRCS x2/x2_init.c x2/x2_io.c x2/x2_memory.c x2/x2_subcore.c)
set(X3 x3)
set(X3_SRCS x3/x3_init.c x3/x3_io.c x3/x3_memory.c x3/x3_subcore.c)
set(XP xp)
set(XP_SRCS xp/xp_init.c xp/xp_io.c xp/xp_memory.c xp/xp_subcore.c)
set(XS xs)
set(XS_SRCS xs/xs_init.c xs/xs_io.c xs/xs_memory.c xs/xs_subcore.c)
set(X4 x4)
set(X4_SRCS x4/x4_init.c x4/x4_io.c x4/x4_memory.c x4/x4_subcore.c)
set(XZ xz)
set(XZ_SRCS xz/xz_init.c xz/xz_io.c xz/xz_memory.c xz/xz_subcore.c)
set(XN xn)
set(XN_SRCS xn/xn_init.c xn/xn_io.c xn/xn_memory.c xn/xn_subcore.c)
set(XC xc)
set(XC_SRCS xc/xc_init.c xc/xc_io.c xc/xc_memory.c xc/xc_lcd.c xc/xc_subcore.c)
set(X5 x5)
set(X5_SRCS x5/x5_init.c x5/x5_io.c x5/x5_memory.c x5/x5_subcore.c)
set(X6 x6)
set(X6_SRCS x6/x6_init.c x6/x6_io.c x6/x6_memory.c x6/x6_subcore.c)

add_definitions( -DHAVE_CONFIG_H )

add_library(${EMU_TARGET} STATIC
    ${CALCS_SRCS}
    ${Z80_SRCS} ${STATE_SRCS} ${ROM_SRCS} ${FLASH_SRCS}
    ${LINK_SRCS} ${KEYPAD_SRCS} ${LCD_SRCS} ${CERT_SRCS} ${MD5_SRCS}
    ${TIMERS_SRCS} ${MONOLCD_SRCS} ${GRAYLCD_SRCS} ${GRAYIMAGE_SRCS}
    ${GRAYCOLOR_SRCS} ${AUDIO_SRCS}
    ${X7_SRCS} ${X1_SRCS} ${X2_SRCS} ${X3_SRCS} ${XP_SRCS} ${XS_SRCS}
    ${X4_SRCS} ${XZ_SRCS} ${XN_SRCS} ${XC_SRCS} ${X5_SRCS} ${X6_SRCS})

target_link_libraries(${EMU_TARGET}
    m
    ${Glib_LIBRARIES} ${TiConv_LIBRARIES} ${TiCalcs2_LIBRARIES})

#set(CMAKE_C_FLAGS "-Wall -std=gnu99 -fPIC")
#set(CMAKE_C_FLAGS_DEBUG "-Wall -std=gnu99 -fPIC")
#set(CMAKE_C_FLAGS_RELEASE "-Wall -std=gnu99 -fPIC")

file (GLOB HEADER_FILES *.h)
install (FILES ${HEADER_FILES} DESTINATION ${INCLUDE_INSTALL_DIR})
