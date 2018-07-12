/***********************************************************************
* ******************************************************************** *
* *                                                                  * *
* *   BIOS Selector Shell for Rabbit 2000 and 3000 CPU based boards. * *
* *                                                                  * *
* *                                                                  * *
* *    Copyright (c)  1999-2006 Rabbit Semiconductor, Inc.           * *
* *                                                                  * *
* *   Assumes ATMEL compatible flash algorithms.                     * *
* *                                                                  * *
* ******************************************************************** *
***********************************************************************/

#ifndef __BIOS_ALREADY_COMPILED
   #define __BIOS_ALREADY_COMPILED
#else
   #fatal "Cannot compile the BIOS as a user program!"
#endif

#use "BOARDTYPES.LIB"	// board-specific initialization header
#use "SYSCONFIG.LIB"		// All user-defined macros for BIOS configuration
#use "SYSIODEFS.LIB"

#if __RABBITSYS && !FAST_RAM_COMPILE
      #fatal "Select Code and BIOS in Flash, Run in RAM for RabbitSys in the Compiler Project Options."
#endif

enum OriginType { UNKNOWN_ORG = 0, RCODORG, XCODORG, WCODORG, RVARORG, XVARORG,
						WVARORG, RCONORG, RESVORG };

typedef struct {
	char type;
   char flags;                //
   unsigned long	paddr;      // Physical address
   unsigned		laddr;      	// Logical address
   unsigned long 	usedbytes;  // Actual number of bytes used or reserved starting from paddr/laddr
   unsigned long  totalbytes; // Total space allocated
} _sys_mem_origin_t;

// These are defined by the compiler.
extern int _orgtablesize;
extern _sys_mem_origin_t _orgtable[];

#define _cexpr(X) 0+(X)

#if __RABBITSYS == 0
   #use "stdBIOS.c"
#elif __RABBITSYS == 1
	#use "sysBIOS.c"
#else
   #use "rkBIOS.c"
#endif


#if !_USER
#flushlib
#flushlib

#pragma CompileProgram

#flushcompile

#flushlib
#flushlib
#flushlib

#asm
StartUserCode::
#ifdef RFU_BIN_WAIT_FOR_RESET
 #if _DK_ENABLE_DEBUGKERNEL_
	#fatal "Undefine RFU_BIN_WAIT_FOR_RESET when the debugger is enabled."
 #endif
		push	af
ioi	ld		a, (SPCR)
		and	0x60
		jr		z, .rfu_continue_after_reset

.rfu_wait_for_reset:
		call	bioshitwd
		jr		.rfu_wait_for_reset

.rfu_continue_after_reset:
		pop	af
#else
 #ifdef RFU_BIN_WAIT_FOR_RUN_MODE
  #if _DK_ENABLE_DEBUGKERNEL_
	#fatal "Undefine RFU_BIN_WAIT_FOR_RUN_MODE when the debugger is enabled."
  #endif
		push	af
.rfu_wait_for_run_mode:
		call	bioshitwd
ioi	ld		a, (SPCR)
		and	0x60
		jr		nz, .rfu_wait_for_run_mode

		pop	af
 #endif	// RFU_BIN_WAIT_FOR_RUN_MODE
#endif	// RFU_BIN_WAIT_FOR_RESET
#endasm

#if __SEPARATE_INST_DATA__
#asm xmem
XStartUserCode::
#endasm
#endif
#endif


// NOTE:  The #use "PRECOMPILE.LIB" MUST come after the #pragma CompileProgram.
#use "PRECOMPILE.LIB"