/* There 's a lot of different method to debug */
/* To keep the code clean, I choose to define macros */
/* simply add -DDEBUGGINGCLICK_L0 (just an example) into the Makefile to activate debug infos for click function and level 0 */
/* Add level 2 for complete info ;D but it will print a LOT of message (refresh lcd by example will send you 10 msg by sec) */
/* DEBUGGING****_L*_A*    ----> **** for the kind of info ,   L* for the level , A*  for the number of arguments */

#ifdef DEBUG
#define DEBUGGING(x)  printf(x)
#define DEBUGGING2(x,y)  printf(x,y)
#define DEBUGGING3(x,y,z)  printf(x,y,z)
#else
#define DEBUGGING(x)
#define DEBUGGING2(x,y)
#define DEBUGGING3(x,y,z)
#endif

#ifdef DEBUGGINGCLICK_L0
#define DEBUGGINGCLICK_L0_A0(x)  printf(x)
#define DEBUGGINGCLICK_L0_A1(x,y)  printf(x,y)
#define DEBUGGINGCLICK_L0_A2(x,y,z)  printf(x,y,z)
#define DEBUGGINGCLICK_L0_A3(x,y,z,t)  printf(x,y,z,t)
#define DEBUGGINGCLICK_L0_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DEBUGGINGCLICK_L0_A5(x,y,z,t,u,v) printf(x,y,z,t,u,v)
#else
#define DEBUGGINGCLICK_L0_A0(x)
#define DEBUGGINGCLICK_L0_A1(x,y)
#define DEBUGGINGCLICK_L0_A2(x,y,z)
#define DEBUGGINGCLICK_L0_A3(x,y,z,t)
#define DEBUGGINGCLICK_L0_A4(x,y,z,t,u)
#define DEBUGGINGCLICK_L0_A5(x,y,z,t,u,v)
#endif

#ifdef DEBUGGINGCLICK_L2
#define DEBUGGINGCLICK_L2_A0(x)  printf(x)
#define DEBUGGINGCLICK_L2_A1(x,y)  printf(x,y)
#define DEBUGGINGCLICK_L2_A2(x,y,z)  printf(x,y,z)
#define DEBUGGINGCLICK_L2_A3(x,y,z,t)  printf(x,y,z,t)
#define DEBUGGINGCLICK_L2_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DEBUGGINGCLICK_L2_A5(x,y,z,t,u,v)  printf(x,y,z,t,u,v)
#else
#define DEBUGGINGCLICK_L2_A0(x)
#define DEBUGGINGCLICK_L2_A1(x,y)
#define DEBUGGINGCLICK_L2_A2(x,y,z)
#define DEBUGGINGCLICK_L2_A3(x,y,z,t)
#define DEBUGGINGCLICK_L2_A4(x,y,z,t,u)
#define DEBUGGINGCLICK_L2_A5(x,y,z,t,u,v)
#endif

#ifdef DEBUGGINGLCD_L0
#define DEBUGGINGLCD_L0_A0(x)  printf(x)
#define DEBUGGINGLCD_L0_A1(x,y)  printf(x,y)
#define DEBUGGINGLCD_L0_A2(x,y,z)  printf(x,y,z)
#define DEBUGGINGLCD_L0_A3(x,y,z,t)  printf(x,y,z,t)
#define DEBUGGINGLCD_L0_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DEBUGGINGLCD_L0_A5(x,y,z,t,u,v)  printf(x,y,z,t,u,v)
#else
#define DEBUGGINGLCD_L0_A0(x)
#define DEBUGGINGLCD_L0_A1(x,y)
#define DEBUGGINGLCD_L0_A2(x,y,z)
#define DEBUGGINGLCD_L0_A3(x,y,z,t)
#define DEBUGGINGLCD_L0_A4(x,y,z,t,u)
#define DEBUGGINGLCD_L0_A5(x,y,z,t,u,v)
#endif

#ifdef DEBUGGINGLCD_L2
#define DEBUGGINGLCD_L2_A0(x)  printf(x)
#define DEBUGGINGLCD_L2_A1(x,y)  printf(x,y)
#define DEBUGGINGLCD_L2_A2(x,y,z)  printf(x,y,z)
#define DEBUGGINGLCD_L2_A3(x,y,z,t)  printf(x,y,z,t)
#define DEBUGGINGLCD_L2_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DEBUGGINGLCD_L2_A5(x,y,z,t,u,v)  printf(x,y,z,t,u,v)
#else
#define DEBUGGINGLCD_L2_A0(x)
#define DEBUGGINGLCD_L2_A1(x,y)
#define DEBUGGINGLCD_L2_A2(x,y,z)
#define DEBUGGINGLCD_L2_A3(x,y,z,t)
#define DEBUGGINGLCD_L2_A4(x,y,z,t,u)
#define DEBUGGINGLCD_L2_A5(x,y,z,t,u,v)
#endif

#ifdef DEBUGGINGGLOBAL_L0
#define DEBUGGINGGLOBAL_L0_A0(x)  printf(x)
#define DEBUGGINGGLOBAL_L0_A1(x,y)  printf(x,y)
#define DEBUGGINGGLOBAL_L0_A2(x,y,z)  printf(x,y,z)
#define DEBUGGINGGLOBAL_L0_A3(x,y,z,t)  printf(x,y,z,t)
#define DEBUGGINGGLOBAL_L0_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DEBUGGINGGLOBAL_L0_A5(x,y,z,t,u,v)  printf(x,y,z,t,u,v)
#else
#define DEBUGGINGGLOBAL_L0_A0(x)
#define DEBUGGINGGLOBAL_L0_A1(x,y)
#define DEBUGGINGGLOBAL_L0_A2(x,y,z)
#define DEBUGGINGGLOBAL_L0_A3(x,y,z,t)
#define DEBUGGINGGLOBAL_L0_A4(x,y,z,t,u)
#define DEBUGGINGGLOBAL_L0_A5(x,y,z,t,u,v)
#endif

#ifdef DEBUGGINGGLOBAL_L2
#define DEBUGGINGGLOBAL_L2_A0(x)  printf(x)
#define DEBUGGINGGLOBAL_L2_A1(x,y)  printf(x,y)
#define DEBUGGINGGLOBAL_L2_A2(x,y,z)  printf(x,y,z)
#define DEBUGGINGGLOBAL_L2_A3(x,y,z,t)  printf(x,y,z,t)
#define DEBUGGINGGLOBAL_L2_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DEBUGGINGGLOBAL_L2_A5(x,y,z,t,u,v)  printf(x,y,z,t,u,v)
#else
#define DEBUGGINGGLOBAL_L2_A0(x)
#define DEBUGGINGGLOBAL_L2_A1(x,y)
#define DEBUGGINGGLOBAL_L2_A2(x,y,z)
#define DEBUGGINGGLOBAL_L2_A3(x,y,z,t)
#define DEBUGGINGGLOBAL_L2_A4(x,y,z,t,u)
#define DEBUGGINGGLOBAL_L2_A5(x,y,z,t,u,v)
#endif

#ifdef DEBUGGINGSKIN_L0
#define DEBUGGINGSKIN_L0_A0(x)  printf(x)
#define DEBUGGINGSKIN_L0_A1(x,y)  printf(x,y)
#define DEBUGGINGSKIN_L0_A2(x,y,z)  printf(x,y,z)
#define DEBUGGINGSKIN_L0_A3(x,y,z,t)  printf(x,y,z,t)
#define DEBUGGINGSKIN_L0_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DEBUGGINGSKIN_L0_A5(x,y,z,t,u,v)  printf(x,y,z,t,u,v)
#else
#define DEBUGGINGSKIN_L0_A0(x)
#define DEBUGGINGSKIN_L0_A1(x,y)
#define DEBUGGINGSKIN_L0_A2(x,y,z)
#define DEBUGGINGSKIN_L0_A3(x,y,z,t)
#define DEBUGGINGSKIN_L0_A4(x,y,z,t,u)
#define DEBUGGINGSKIN_L0_A5(x,y,z,t,u,v)
#endif

#ifdef DEBUGGINGSKIN_L2
#define DEBUGGINGSKIN_L2_A0(x)  printf(x)
#define DEBUGGINGSKIN_L2_A1(x,y)  printf(x,y)
#define DEBUGGINGSKIN_L2_A2(x,y,z)  printf(x,y,z)
#define DEBUGGINGSKIN_L2_A3(x,y,z,t)  printf(x,y,z,t)
#define DEBUGGINGSKIN_L2_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DEBUGGINGSKIN_L2_A5(x,y,z,t,u,v)  printf(x,y,z,t,u,v)
#else
#define DEBUGGINGSKIN_L2_A0(x)
#define DEBUGGINGSKIN_L2_A1(x,y)
#define DEBUGGINGSKIN_L2_A2(x,y,z)
#define DEBUGGINGSKIN_L2_A3(x,y,z,t)
#define DEBUGGINGSKIN_L2_A4(x,y,z,t,u)
#define DEBUGGINGSKIN_L2_A5(x,y,z,t,u,v)
#endif

/* Here is the part for the config.dat information (creation, reading, writing ...) */
#ifdef DEBUGGINGCONFIG_FILE_L0
#define DEBUGGINGCONFIG_FILE_L0_A0(x)  printf(x)
#define DEBUGGINGCONFIG_FILE_L0_A1(x,y)  printf(x,y)
#define DEBUGGINGCONFIG_FILE_L0_A2(x,y,z)  printf(x,y,z)
#define DEBUGGINGCONFIG_FILE_L0_A3(x,y,z,t)  printf(x,y,z,t)
#define DEBUGGINGCONFIG_FILE_L0_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DEBUGGINGCONFIG_FILE_L0_A5(x,y,z,t,u,v)  printf(x,y,z,t,u,v)
#else
#define DEBUGGINGCONFIG_FILE_L0_A0(x)
#define DEBUGGINGCONFIG_FILE_L0_A1(x,y)
#define DEBUGGINGCONFIG_FILE_L0_A2(x,y,z)
#define DEBUGGINGCONFIG_FILE_L0_A3(x,y,z,t)
#define DEBUGGINGCONFIG_FILE_L0_A4(x,y,z,t,u)
#define DEBUGGINGCONFIG_FILE_L0_A5(x,y,z,t,u,v)
#endif

#ifdef DEBUGGINGCONFIG_FILE_L2
#define DEBUGGINGCONFIG_FILE_L2_A0(x)  printf(x)
#define DEBUGGINGCONFIG_FILE_L2_A1(x,y)  printf(x,y)
#define DEBUGGINGCONFIG_FILE_L2_A2(x,y,z)  printf(x,y,z)
#define DEBUGGINGCONFIG_FILE_L2_A3(x,y,z,t)  printf(x,y,z,t)
#define DEBUGGINGCONFIG_FILE_L2_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DEBUGGINGCONFIG_FILE_L2_A5(x,y,z,t,u,v)  printf(x,y,z,t,u,v)
#else
#define DEBUGGINGCONFIG_FILE_L2_A0(x)
#define DEBUGGINGCONFIG_FILE_L2_A1(x,y)
#define DEBUGGINGCONFIG_FILE_L2_A2(x,y,z)
#define DEBUGGINGCONFIG_FILE_L2_A3(x,y,z,t)
#define DEBUGGINGCONFIG_FILE_L2_A4(x,y,z,t,u)
#define DEBUGGINGCONFIG_FILE_L2_A5(x,y,z,t,u,v)
#endif

#ifdef DEBUGGINGDEBUGGER_L0
#define DEBUGGINGDEBUGGER_L0_A0(x)  printf(x)
#define DEBUGGINGDEBUGGER_L0_A1(x,y)  printf(x,y)
#define DEBUGGINGDEBUGGER_L0_A2(x,y,z)  printf(x,y,z)
#define DEBUGGINGDEBUGGER_L0_A3(x,y,z,t)  printf(x,y,z,t)
#define DEBUGGINGDEBUGGER_L0_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DEBUGGINGDEBUGGER_L0_A5(x,y,z,t,u,v)  printf(x,y,z,t,u,v)
#else
#define DEBUGGINGDEBUGGER_L0_A0(x)
#define DEBUGGINGDEBUGGER_L0_A1(x,y)
#define DEBUGGINGDEBUGGER_L0_A2(x,y,z)
#define DEBUGGINGDEBUGGER_L0_A3(x,y,z,t)
#define DEBUGGINGDEBUGGER_L0_A4(x,y,z,t,u)
#define DEBUGGINGDEBUGGER_L0_A5(x,y,z,t,u,v)
#endif
#ifdef DEBUGGINGDEBUGGER_L2
#define DEBUGGINGDEBUGGER_L2_A0(x)  printf(x)
#define DEBUGGINGDEBUGGER_L2_A1(x,y)  printf(x,y)
#define DEBUGGINGDEBUGGER_L2_A2(x,y,z)  printf(x,y,z)
#define DEBUGGINGDEBUGGER_L2_A3(x,y,z,t)  printf(x,y,z,t)
#define DEBUGGINGDEBUGGER_L2_A4(x,y,z,t,u)  printf(x,y,z,t,u)
#define DEBUGGINGDEBUGGER_L2_A5(x,y,z,t,u,v)  printf(x,y,z,t,u,v)
#else
#define DEBUGGINGDEBUGGER_L2_A0(x)
#define DEBUGGINGDEBUGGER_L2_A1(x,y)
#define DEBUGGINGDEBUGGER_L2_A2(x,y,z)
#define DEBUGGINGDEBUGGER_L2_A3(x,y,z,t)
#define DEBUGGINGDEBUGGER_L2_A4(x,y,z,t,u)
#define DEBUGGINGDEBUGGER_L2_A5(x,y,z,t,u,v)
#endif

