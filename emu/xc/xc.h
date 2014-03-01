/*
 * libtilemcore - Graphing calculator emulation library
 *
 * Copyright (C) 2009-2013 Benjamin Moody
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef _TILEM_XC_H
#define _TILEM_XC_H

enum {
	PORT3,			/* mask of enabled interrupts */
	PORT4,			/* interrupt timer speed */
	PORT5,			/* memory mapping bank C */
	PORT6,			/* memory mapping bank A */
	PORT7,			/* memory mapping bank B */
	PORT8,			/* link assist mode flags */
	PORT9,			/* unknown (link assist settings?) */
	PORTA,			/* unknown (timeout value?) */
	PORTB,			/* unknown (timeout value?) */
	PORTC,			/* unknown (timeout value?) */
	PORTD,			/* unknown */
	PORTE,			/* memory mapping bank A extension */
	PORTF,			/* memory mapping bank B extension */

	PORT20,			/* CPU speed control */
	PORT21,			/* hardware type / RAM no-exec control */
	PORT22,			/* Flash no-exec lower limit */
	PORT23,			/* Flash no-exec upper limit */
	PORT24,			/* Flash no-exec limit bit 9 */
	PORT25,			/* RAM no-exec lower limit */
	PORT26,			/* RAM no-exec upper limit */
	PORT27,			/* bank C forced-page-0 limit */
	PORT28,			/* bank B forced-page-1 limit */
	PORT29,			/* LCD port delay (6 MHz) */
	PORT2A,			/* LCD port delay (mode 1) */
	PORT2B,			/* LCD port delay (mode 2) */
	PORT2C,			/* LCD port delay (mode 3) */
	PORT2D,			/* unknown */
	PORT2E,			/* memory delay */
	PORT2F,			/* Duration of LCD wait timer */
	PORT39,			/* misc hardware control */
	PORT3A,			/* misc hardware control */

	CLOCK_MODE,		/* clock mode */
	CLOCK_INPUT,		/* clock input register */
	CLOCK_DIFF,		/* clock value minus actual time */

	RAM_READ_DELAY,
	RAM_WRITE_DELAY,
	RAM_EXEC_DELAY,
	FLASH_READ_DELAY,
	FLASH_WRITE_DELAY,
	FLASH_EXEC_DELAY,
	LCD_PORT_DELAY,
	NO_EXEC_RAM_MASK,
	NO_EXEC_RAM_LOWER,
	NO_EXEC_RAM_UPPER,

	BACKLIGHT_ON,
	BACKLIGHT_LEVEL,

	LCD_READ_STATE,
	LCD_READ_BUFFER,
	LCD_WRITE_STATE,
	LCD_WRITE_BUFFER,
	LCD_REG_INDEX,
	LCD_R01, LCD_R02, LCD_R03, LCD_R04,
	LCD_R05, LCD_R06, LCD_R07, LCD_R08,
	LCD_R09, LCD_R0A, LCD_R0C, LCD_R0D,
	LCD_R0F, LCD_R10, LCD_R11, LCD_R12,
	LCD_R13, LCD_R20, LCD_R21, LCD_R29,
	LCD_R2B, LCD_R30, LCD_R31, LCD_R32,
	LCD_R35, LCD_R36, LCD_R37, LCD_R38,
	LCD_R39, LCD_R3C, LCD_R3D, LCD_R50,
	LCD_R51, LCD_R52, LCD_R53, LCD_R60,
	LCD_R61, LCD_R6A, LCD_R80, LCD_R81,
	LCD_R82, LCD_R83, LCD_R84, LCD_R85,
	LCD_R90, LCD_R92, LCD_R95, LCD_R97,
	LCD_RA1, LCD_RA2, LCD_RA3, LCD_RA4,
	LCD_RA5, LCD_RE2, LCD_RE3, LCD_RE4,
	LCD_RE5, LCD_RE6, LCD_RE7, LCD_RE9,
	LCD_REA, LCD_REB, LCD_REC, LCD_RED,
	LCD_REE, LCD_REF, LCD_RFE, LCD_RFF,
	LCD_CUR_X, LCD_CUR_Y,

	LCD_WAIT,		/* LCD wait timer active */
	PROTECTSTATE,		/* port protection state */
	NUM_HW_REGS
};

#define HW_REG_NAMES \
	{ "port3", "port4", "port5", "port6", "port7", "port8", "port9", \
	  "portA", "portB", "portC", "portD", "portE", "portF", "port20", \
	  "port21", "port22", "port23", "port24", "port25", "port26", "port27", \
	  "port28", "port29", "port2A", "port2B", "port2C", "port2D", \
	  "port2E", "port2F", "port39", "port3A", \
	  "clock_mode", "clock_input", "clock_diff", \
	  "ram_read_delay", "ram_write_delay", "ram_exec_delay", \
	  "flash_read_delay", "flash_write_delay", "flash_exec_delay", \
	  "lcd_port_delay", "no_exec_ram_mask", "no_exec_ram_lower", \
	  "no_exec_ram_upper", "backlight_on", "backlight_level", \
	  "lcd_read_state", "lcd_read_buffer", \
	  "lcd_write_state", "lcd_write_buffer", "lcd_reg_index", \
	  "lcd_r01", "lcd_r02", "lcd_r03", "lcd_r04", \
	  "lcd_r05", "lcd_r06", "lcd_r07", "lcd_r08", \
	  "lcd_r09", "lcd_r0a", "lcd_r0c", "lcd_r0d", \
	  "lcd_r0f", "lcd_r10", "lcd_r11", "lcd_r12", \
	  "lcd_r13", "lcd_r20", "lcd_r21", "lcd_r29", \
	  "lcd_r2b", "lcd_r30", "lcd_r31", "lcd_r32", \
	  "lcd_r35", "lcd_r36", "lcd_r37", "lcd_r38", \
	  "lcd_r39", "lcd_r3c", "lcd_r3d", "lcd_r50", \
	  "lcd_r51", "lcd_r52", "lcd_r53", "lcd_r60", \
	  "lcd_r61", "lcd_r6a", "lcd_r80", "lcd_r81", \
	  "lcd_r82", "lcd_r83", "lcd_r84", "lcd_r85", \
	  "lcd_r90", "lcd_r92", "lcd_r95", "lcd_r97", \
	  "lcd_ra1", "lcd_ra2", "lcd_ra3", "lcd_ra4", \
	  "lcd_ra5", "lcd_re2", "lcd_re3", "lcd_re4", \
	  "lcd_re5", "lcd_re6", "lcd_re7", "lcd_re9", \
	  "lcd_rea", "lcd_reb", "lcd_rec", "lcd_red", \
	  "lcd_ree", "lcd_ref", "lcd_rfe", "lcd_rff", \
	  "lcd_cur_x", "lcd_cur_y", "lcd_wait", "protectstate" }

#define TIMER_INT1 (TILEM_NUM_SYS_TIMERS + 1)
#define TIMER_INT2A (TILEM_NUM_SYS_TIMERS + 2)
#define TIMER_INT2B (TILEM_NUM_SYS_TIMERS + 3)
#define TIMER_LCD_WAIT (TILEM_NUM_SYS_TIMERS + 4)
#define TIMER_BACKLIGHT_OFF (TILEM_NUM_SYS_TIMERS + 5)
#define NUM_HW_TIMERS 5

#define HW_TIMER_NAMES { "int1", "int2a", "int2b", "lcd_wait", "backlight_off" }

void xc_reset(TilemCalc* calc);
void xc_stateloaded(TilemCalc* calc, int savtype);
byte xc_z80_in(TilemCalc* calc, dword port);
void xc_z80_out(TilemCalc* calc, dword port, byte value);
void xc_z80_ptimer(TilemCalc* calc, int id);
void xc_z80_wrmem(TilemCalc* calc, dword addr, byte value);
byte xc_z80_rdmem(TilemCalc* calc, dword addr);
byte xc_z80_rdmem_m1(TilemCalc* calc, dword addr);
dword xc_mem_ltop(TilemCalc* calc, dword addr);
dword xc_mem_ptol(TilemCalc* calc, dword addr);
void xc_lcd_control(TilemCalc* calc, byte val);
byte xc_lcd_read(TilemCalc* calc);
void xc_lcd_write(TilemCalc* calc, byte val);
void xc_get_lcd(TilemCalc* calc, byte* data);
void xc_get_frame(TilemCalc* calc, TilemLCDBuffer* buf);

#endif
