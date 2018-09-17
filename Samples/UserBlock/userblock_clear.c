/*
   Copyright (c) 2015, Digi International Inc.

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
/***********************************************************************
	userblock_clear.c

	This program clears the contents of the user block.  Note that it
	does NOT clear the calibration constants or the system ID block.
	After writing, the program checks to make sure fill pattern is there.

	See userblock_info.c for general information about your board's
	installed User Block.

	Note:  In Debug mode, there is a 300 millisecond timer delay after
	       each writeUserBlock call to allow Dynamic C and the target
	       controller to exchange a debug packet, in order to inform the
	       debug kernel that the target is still alive.  The timer delay
          is not necessary in nodebug Run mode or when single-stepping.

***********************************************************************/

#class auto

#if _USER
	#error "User block access not provided under RabbitSys."
#endif

// If it exists, use the Z-World reserved size that is explicitly set
//  for this controller in its hardware-specific library.
#ifndef ZWORLD_RESERVED_SIZE
	// Otherwise, reserve 2K bytes of space below top of ID/User Blocks.
	// This may be overprotective for boards with no calibration constants
	//  or other Z-World installed information, but, better safe than sorry!
	#define ZWORLD_RESERVED_SIZE 0x0800
	#define ASSUMED_SIZE_STRING " (assumed)"
#else
	#define ASSUMED_SIZE_STRING ""
#endif

#define CLEAR_VALUE 0xFF

// local function prototypes
void delay_ms(unsigned long delay_mS);
unsigned long GetAvailableUserBlockSize(void);
void lockup(void);
unsigned long MaySubtractIDBlockSize(unsigned long available_size);
unsigned long MaySubtractReservedSize(unsigned long available_size);

void main(void)
{
	static char buffer[4096];			// Our 4K stack will blow if "auto"
	auto int err;
   auto unsigned ub_rwcheck, ub_rwsize;
	auto unsigned long available_size, ub_rwaddr;

	// Initialize the buffer
	memset(buffer, CLEAR_VALUE, sizeof(buffer));

	// as the very long descriptive name says . . .
	available_size = GetAvailableUserBlockSize();

	printf("The available User block size is 0x%08LX (%ld) bytes.\n",
	       available_size, available_size);
	printf("Z-World's%s reserved size is 0x%08LX (%ld) bytes.\n\n",
	       ASSUMED_SIZE_STRING, (unsigned long) (ZWORLD_RESERVED_SIZE),
	       (unsigned long) (ZWORLD_RESERVED_SIZE));

	/*  Write a pattern into the User Block: */
	ub_rwaddr = 0ul;
	while (ub_rwaddr < available_size) {
		if ((ub_rwaddr + sizeof(buffer)) <= available_size) {
			ub_rwsize = sizeof(buffer);
		} else {
			ub_rwsize = (unsigned) (available_size - ub_rwaddr);
		}
		err = writeUserBlock((unsigned) ub_rwaddr, buffer, ub_rwsize);
		if (OPMODE & 0x08) {
			delay_ms(300ul);
		}
		if (err) {
			printf("Error %d:  Pattern write failed, offset = 0x%08LX.\n",
			       err, ub_rwaddr);
			lockup();
		} else {
			printf("Wrote 0x%04X bytes to User block, offset = 0x%08LX.\n",
			       ub_rwsize, ub_rwaddr);
		}
		ub_rwaddr += ub_rwsize;
	}

	printf("\n");

	/*  Now read back and ensure the pattern was written, abort if not: */
	ub_rwaddr = 0ul;
	while (ub_rwaddr < available_size) {
		if ((ub_rwaddr + sizeof(buffer)) <= available_size) {
			ub_rwsize = sizeof(buffer);
		} else {
			ub_rwsize = (unsigned) (available_size - ub_rwaddr);
		}
		memset(buffer, ~CLEAR_VALUE, sizeof(buffer));
		err = readUserBlock(buffer, (unsigned) ub_rwaddr, ub_rwsize);
		if (err) {
			printf("Error %d:  Pattern read failed, offset = 0x%08LX.\n",
			       err, ub_rwaddr);
			lockup();
		} else {
			printf("Read 0x%04X bytes from User block, offset = 0x%08LX.\n",
			       ub_rwsize, ub_rwaddr);
		}
		for (ub_rwcheck = 0; ub_rwcheck < ub_rwsize; ub_rwcheck++) {
			if (buffer[ub_rwcheck] != (char) CLEAR_VALUE) {
				printf("Error:  Pattern didn't stick, offset = 0x%08LX.\n",
				       ub_rwaddr + ub_rwcheck);
				lockup();
			}
		}
		ub_rwaddr += ub_rwsize;
	}

	if (available_size > 0L) {
		printf("\n\nAvailable User block area cleared!\n");
	} else {
		printf("\n\nDid not attempt to clear the available User block area!\n");
		printf("Either no User block installed, or bad User block information!\n");
		printf("Recommend running the userblock_info.c sample program.\n");
	}
   lockup();
}

void delay_ms(unsigned long delay_mS)
{
	auto unsigned long delay_end;

	delay_end = MS_TIMER + delay_mS;
	// OK to return early if MS_TIMER + delay_mS wraps
	while ((MS_TIMER < delay_end) && (delay_end >= delay_mS));
}

unsigned long GetAvailableUserBlockSize(void)
{
	auto unsigned long available_size;

	// first, get the total User block size
	//  (NB:  possibly combined with the ID block)
	available_size = GetIDBlockSize() * 4096ul;

	// next, protect the User Block as necessary
	if ((5 == SysIDBlock.tableVersion) && !SysIDBlock.userBlockSize
	    && (SysIDBlock.idBlock2.userBlockSiz2 > 0ul))
	{
		// version 5 "new style" unique ID block w/ mirrored, separate User blocks
		if (!ZWORLD_RESERVED_SIZE) {
			available_size = SysIDBlock.idBlock2.userBlockSiz2;
		} else {
			available_size = MaySubtractReservedSize(available_size);
		}
	}
	else if (((5 == SysIDBlock.tableVersion) && (SysIDBlock.userBlockSize > 0ul))
	         || ((4 >= SysIDBlock.tableVersion)
	             && (2 <= SysIDBlock.tableVersion)))
	{
		// version 5 "old style" mirrored, combined ID + User blocks
		// version 4 mirrored, combined ID + User blocks
		// version 3 mirrored, combined ID + User blocks
		// version 2 unique ID block possibly combined w/ a User block
		available_size = MaySubtractIDBlockSize(available_size);
		available_size = MaySubtractReservedSize(available_size);
	}
	else if ((1 >= SysIDBlock.tableVersion) && (0 <= SysIDBlock.tableVersion)) {
		// version 1 unique ID block possibly w/ a separate simulated User block
		// version 0, no ID block but possibly a single simulated User block
		available_size = MaySubtractReservedSize(available_size);
	} else {
		// either zero-sized User block or unknown ID block version,
		//  protect the entire User Block!
		available_size = 0ul;
	}
	return available_size;
}

void lockup(void)
{
	printf("\n\nLocking up to prevent repetitive flash write attempts.\n");
	printf("In Debug mode, press Ctrl-Q and then Ctrl-F2 to reset this program.\n");
	for(;;);
}

unsigned long MaySubtractIDBlockSize(unsigned long available_size)
{
	if (!ZWORLD_RESERVED_SIZE) {
		if (available_size > SysIDBlock.idBlockSize) {
			available_size -= SysIDBlock.idBlockSize;
		} else {
			// either the User block is improperly defined or some error has
			//  occurred, protect the entire User Block!
			available_size = 0ul;
		}
	}
	return available_size;
}

unsigned long MaySubtractReservedSize(unsigned long available_size)
{
	if (available_size > ZWORLD_RESERVED_SIZE) {
		// protect the calibration constants, etc.
		available_size -= ZWORLD_RESERVED_SIZE;
	} else {
		// either no User block is defined or some error has occurred,
		//  protect the entire User Block!
		available_size = 0ul;
	}
	return available_size;
}

