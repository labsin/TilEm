/*
 * TilEm II
 *
 * Copyright (c) 2010-2011 Thibault Duponchelle
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* There 's a lot of different method to debug */
/* To keep the code clean, I choose to define macros */
/* simply add -DDCLICK_L0 (just an example) into the Makefile to activate debug infos for click function and level 0 */
/* Add level 2 for complete info ;D but it will print a LOT of message (refresh lcd by example will send you 10 msg by sec) */
/* D****_L*_A*    ----> **** for the kind of info ,   L* for the level , A*  for the number of arguments */

#ifdef D1
#define DGLOBAL_L0 
#define DGLOBAL_L2 
#define DSKIN_L0 
#define DCLICK_L0 
#define DCLICK_L2 
#define DDEBUGGER_L0 
#define DMACRO_L0
#endif

#ifdef DEBUG
#define DEBUGGING(x)  printf(x)
#define DEBUGGING2(x,y)  printf(x,y)
#define DEBUGGING3(x,y,z)  printf(x,y,z)
#else
#define DEBUGGING(x)
#define DEBUGGING2(x,y)
#define DEBUGGING3(x,y,z)
#endif

#ifdef DCLICK_L0
#define DCLICK_L0_A0(x)  printf(x)
#define DCLICK_L0_A1(x,y)  printf(x,y)
#define DCLICK_L0_A2(x,y,z)  printf(x,y,z)
#define DCLICK_L0_A3(x,y,z,t)  printf(x,y,z,t)
#define DCLICK_L0_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DCLICK_L0_A5(x,y,z,t,u,v) printf(x,y,z,t,u,v)
#else
#define DCLICK_L0_A0(x)
#define DCLICK_L0_A1(x,y)
#define DCLICK_L0_A2(x,y,z)
#define DCLICK_L0_A3(x,y,z,t)
#define DCLICK_L0_A4(x,y,z,t,u)
#define DCLICK_L0_A5(x,y,z,t,u,v)
#endif

#ifdef DCLICK_L2
#define DCLICK_L2_A0(x)  printf(x)
#define DCLICK_L2_A1(x,y)  printf(x,y)
#define DCLICK_L2_A2(x,y,z)  printf(x,y,z)
#define DCLICK_L2_A3(x,y,z,t)  printf(x,y,z,t)
#define DCLICK_L2_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DCLICK_L2_A5(x,y,z,t,u,v)  printf(x,y,z,t,u,v)
#else
#define DCLICK_L2_A0(x)
#define DCLICK_L2_A1(x,y)
#define DCLICK_L2_A2(x,y,z)
#define DCLICK_L2_A3(x,y,z,t)
#define DCLICK_L2_A4(x,y,z,t,u)
#define DCLICK_L2_A5(x,y,z,t,u,v)
#endif

#ifdef DLCD_L0
#define DLCD_L0_A0(x)  printf(x)
#define DLCD_L0_A1(x,y)  printf(x,y)
#define DLCD_L0_A2(x,y,z)  printf(x,y,z)
#define DLCD_L0_A3(x,y,z,t)  printf(x,y,z,t)
#define DLCD_L0_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DLCD_L0_A5(x,y,z,t,u,v)  printf(x,y,z,t,u,v)
#else
#define DLCD_L0_A0(x)
#define DLCD_L0_A1(x,y)
#define DLCD_L0_A2(x,y,z)
#define DLCD_L0_A3(x,y,z,t)
#define DLCD_L0_A4(x,y,z,t,u)
#define DLCD_L0_A5(x,y,z,t,u,v)
#endif

#ifdef DLCD_L2
#define DLCD_L2_A0(x)  printf(x)
#define DLCD_L2_A1(x,y)  printf(x,y)
#define DLCD_L2_A2(x,y,z)  printf(x,y,z)
#define DLCD_L2_A3(x,y,z,t)  printf(x,y,z,t)
#define DLCD_L2_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DLCD_L2_A5(x,y,z,t,u,v)  printf(x,y,z,t,u,v)
#else
#define DLCD_L2_A0(x)
#define DLCD_L2_A1(x,y)
#define DLCD_L2_A2(x,y,z)
#define DLCD_L2_A3(x,y,z,t)
#define DLCD_L2_A4(x,y,z,t,u)
#define DLCD_L2_A5(x,y,z,t,u,v)
#endif

#ifdef DGLOBAL_L0
#define DGLOBAL_L0_A0(x)  printf(x)
#define DGLOBAL_L0_A1(x,y)  printf(x,y)
#define DGLOBAL_L0_A2(x,y,z)  printf(x,y,z)
#define DGLOBAL_L0_A3(x,y,z,t)  printf(x,y,z,t)
#define DGLOBAL_L0_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DGLOBAL_L0_A5(x,y,z,t,u,v)  printf(x,y,z,t,u,v)
#else
#define DGLOBAL_L0_A0(x)
#define DGLOBAL_L0_A1(x,y)
#define DGLOBAL_L0_A2(x,y,z)
#define DGLOBAL_L0_A3(x,y,z,t)
#define DGLOBAL_L0_A4(x,y,z,t,u)
#define DGLOBAL_L0_A5(x,y,z,t,u,v)
#endif

#ifdef DGLOBAL_L2
#define DGLOBAL_L2_A0(x)  printf(x)
#define DGLOBAL_L2_A1(x,y)  printf(x,y)
#define DGLOBAL_L2_A2(x,y,z)  printf(x,y,z)
#define DGLOBAL_L2_A3(x,y,z,t)  printf(x,y,z,t)
#define DGLOBAL_L2_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DGLOBAL_L2_A5(x,y,z,t,u,v)  printf(x,y,z,t,u,v)
#else
#define DGLOBAL_L2_A0(x)
#define DGLOBAL_L2_A1(x,y)
#define DGLOBAL_L2_A2(x,y,z)
#define DGLOBAL_L2_A3(x,y,z,t)
#define DGLOBAL_L2_A4(x,y,z,t,u)
#define DGLOBAL_L2_A5(x,y,z,t,u,v)
#endif

#ifdef DSKIN_L0
#define DSKIN_L0_A0(x)  printf(x)
#define DSKIN_L0_A1(x,y)  printf(x,y)
#define DSKIN_L0_A2(x,y,z)  printf(x,y,z)
#define DSKIN_L0_A3(x,y,z,t)  printf(x,y,z,t)
#define DSKIN_L0_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DSKIN_L0_A5(x,y,z,t,u,v)  printf(x,y,z,t,u,v)
#else
#define DSKIN_L0_A0(x)
#define DSKIN_L0_A1(x,y)
#define DSKIN_L0_A2(x,y,z)
#define DSKIN_L0_A3(x,y,z,t)
#define DSKIN_L0_A4(x,y,z,t,u)
#define DSKIN_L0_A5(x,y,z,t,u,v)
#endif

#ifdef DSKIN_L2
#define DSKIN_L2_A0(x)  printf(x)
#define DSKIN_L2_A1(x,y)  printf(x,y)
#define DSKIN_L2_A2(x,y,z)  printf(x,y,z)
#define DSKIN_L2_A3(x,y,z,t)  printf(x,y,z,t)
#define DSKIN_L2_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DSKIN_L2_A5(x,y,z,t,u,v)  printf(x,y,z,t,u,v)
#else
#define DSKIN_L2_A0(x)
#define DSKIN_L2_A1(x,y)
#define DSKIN_L2_A2(x,y,z)
#define DSKIN_L2_A3(x,y,z,t)
#define DSKIN_L2_A4(x,y,z,t,u)
#define DSKIN_L2_A5(x,y,z,t,u,v)
#endif

/* Here is the part for the config.dat information (creation, reading, writing ...) */
#ifdef DCONFIG_FILE_L0
#define DCONFIG_FILE_L0_A0(x)  printf(x)
#define DCONFIG_FILE_L0_A1(x,y)  printf(x,y)
#define DCONFIG_FILE_L0_A2(x,y,z)  printf(x,y,z)
#define DCONFIG_FILE_L0_A3(x,y,z,t)  printf(x,y,z,t)
#define DCONFIG_FILE_L0_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DCONFIG_FILE_L0_A5(x,y,z,t,u,v)  printf(x,y,z,t,u,v)
#else
#define DCONFIG_FILE_L0_A0(x)
#define DCONFIG_FILE_L0_A1(x,y)
#define DCONFIG_FILE_L0_A2(x,y,z)
#define DCONFIG_FILE_L0_A3(x,y,z,t)
#define DCONFIG_FILE_L0_A4(x,y,z,t,u)
#define DCONFIG_FILE_L0_A5(x,y,z,t,u,v)
#endif

#ifdef DCONFIG_FILE_L2
#define DCONFIG_FILE_L2_A0(x)  printf(x)
#define DCONFIG_FILE_L2_A1(x,y)  printf(x,y)
#define DCONFIG_FILE_L2_A2(x,y,z)  printf(x,y,z)
#define DCONFIG_FILE_L2_A3(x,y,z,t)  printf(x,y,z,t)
#define DCONFIG_FILE_L2_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DCONFIG_FILE_L2_A5(x,y,z,t,u,v)  printf(x,y,z,t,u,v)
#else
#define DCONFIG_FILE_L2_A0(x)
#define DCONFIG_FILE_L2_A1(x,y)
#define DCONFIG_FILE_L2_A2(x,y,z)
#define DCONFIG_FILE_L2_A3(x,y,z,t)
#define DCONFIG_FILE_L2_A4(x,y,z,t,u)
#define DCONFIG_FILE_L2_A5(x,y,z,t,u,v)
#endif

#ifdef DDEBUGGER_L0
#define DDEBUGGER_L0_A0(x)  printf(x)
#define DDEBUGGER_L0_A1(x,y)  printf(x,y)
#define DDEBUGGER_L0_A2(x,y,z)  printf(x,y,z)
#define DDEBUGGER_L0_A3(x,y,z,t)  printf(x,y,z,t)
#define DDEBUGGER_L0_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DDEBUGGER_L0_A5(x,y,z,t,u,v)  printf(x,y,z,t,u,v)
#else
#define DDEBUGGER_L0_A0(x)
#define DDEBUGGER_L0_A1(x,y)
#define DDEBUGGER_L0_A2(x,y,z)
#define DDEBUGGER_L0_A3(x,y,z,t)
#define DDEBUGGER_L0_A4(x,y,z,t,u)
#define DDEBUGGER_L0_A5(x,y,z,t,u,v)
#endif
#ifdef DDEBUGGER_L2
#define DDEBUGGER_L2_A0(x)  printf(x)
#define DDEBUGGER_L2_A1(x,y)  printf(x,y)
#define DDEBUGGER_L2_A2(x,y,z)  printf(x,y,z)
#define DDEBUGGER_L2_A3(x,y,z,t)  printf(x,y,z,t)
#define DDEBUGGER_L2_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DDEBUGGER_L2_A5(x,y,z,t,u,v)  printf(x,y,z,t,u,v)
#else
#define DDEBUGGER_L2_A0(x)
#define DDEBUGGER_L2_A1(x,y)
#define DDEBUGGER_L2_A2(x,y,z)
#define DDEBUGGER_L2_A3(x,y,z,t)
#define DDEBUGGER_L2_A4(x,y,z,t,u)
#define DDEBUGGER_L2_A5(x,y,z,t,u,v)
#endif



#ifdef DMACRO_L0
#define DMACRO_L0_A0(x)  printf(x)
#define DMACRO_L0_A1(x,y)  printf(x,y)
#define DMACRO_L0_A2(x,y,z)  printf(x,y,z)
#define DMACRO_L0_A3(x,y,z,t)  printf(x,y,z,t)
#define DMACRO_L0_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DMACRO_L0_A5(x,y,z,t,u,v)  printf(x,y,z,t,u,v)
#else
#define DMACRO_L0_A0(x)
#define DMACRO_L0_A1(x,y)
#define DMACRO_L0_A2(x,y,z)
#define DMACRO_L0_A3(x,y,z,t)
#define DMACRO_L0_A4(x,y,z,t,u)
#define DMACRO_L0_A5(x,y,z,t,u,v)
#endif
#ifdef DMACRO_L2
#define DMACRO_L2_A0(x)  printf(x)
#define DMACRO_L2_A1(x,y)  printf(x,y)
#define DMACRO_L2_A2(x,y,z)  printf(x,y,z)
#define DMACRO_L2_A3(x,y,z,t)  printf(x,y,z,t)
#define DMACRO_L2_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DMACRO_L2_A5(x,y,z,t,u,v)  printf(x,y,z,t,u,v)
#else
#define DMACRO_L2_A0(x)
#define DMACRO_L2_A1(x,y)
#define DMACRO_L2_A2(x,y,z)
#define DMACRO_L2_A3(x,y,z,t)
#define DMACRO_L2_A4(x,y,z,t,u)
#define DMACRO_L2_A5(x,y,z,t,u,v)
#endif
