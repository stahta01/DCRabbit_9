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
/**************************************************************************
	Simple3wire.c

 	This program is used with PowerCoreFLEX series core modules with the
   prototyping board.

	Description
	===========
	This program demonstrates basic initialization for a
	simple RS232 3-wire loopback displayed in STDIO window.

   Here's the typical connections for a 3 wire interface that
   would be made between controllers.

       TX <---> RX
       RX <---> TX
		Gnd <---> Gnd

	Connections on prototyping board:
	================================
	1. Jumper TXE to RXF together located on J1 pins 3 and 4 with
   	using the 0.1 spacing shunt.
   2. Jumper TXF to RXE together located on J1 pins 5 and 6 with
      using the 0.1 spacing shunt.

	Instructions
	============
	1. Compile and run this program.
 	2. View STDIO window for sample program results.

***************************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// Sample library for PowerCoreFLEX series core modules
#use "PowerCoreFLEX.lib"

// serial buffer size
#define EINBUFSIZE  15
#define EOUTBUFSIZE 15
#define FINBUFSIZE  15
#define FOUTBUFSIZE 15


// serial baud rate
#define BAUD232 115200

main()
{
	auto int nIn;
	auto char cOut;

	// Initialize I/O to use PowerCoreFLEX prototyping board
	brdInit();

   // Open serial port
	serEopen(BAUD232);
   serFopen(BAUD232);

   // Can use serial mode 0 for 3 wire RS232 operation
   // and serMode must be executed after the serXopen
   // function(s).
   serMode(0);

   // Clear serial buffers
	serEwrFlush();
	serErdFlush();
   serFwrFlush();
	serFrdFlush();

	while (1)
	{
		for (cOut='a'; cOut<='z'; ++cOut)
		{
			serEputc (cOut);								//	Send Byte
			while ((nIn=serFgetc()) == -1);			// Wait for echo
			printf ("Upper case %c is %c\n", cOut, toupper(nIn));
		}
	}
}


