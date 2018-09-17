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
/*****************************************************

     Cof EchoBlk.c

     This program echos a block of characters over serial port B.
     It must be run with a serial utility such as Hyperterminal.
     
     Connect RS232 cable from PC to Rabbit:
     	 Connect PC's RS232 GND to Rabbit GND
       Connect PC's RS232 TD  to Rabbit RXB
       Connect PC's RS232 RD  to Rabbit TXB
       
     Configure the serial utility for port connected to RS232, 19200 8N1.
     
     This program uses the single user cofunction form of the serial read
     and write routines, which allows it to cooperatively multi-task with
     other costates when it is not actively reading or writing data. It
     reads bytes over serial port B into a data structure and writes back
     what it read as soon as the number of bytes defined by the length
     parameter is read or when it times out. The length parameter in this
     example is set to the size of the data structure into which the stream
     of serial data will be read. Each time a complete block is read the
     timeout clock stops. Once a new block begins, it will timeout if the
     byte to byte period elapses prior to completing the block. It will
     terminate if a timeout occurs after reading a single <Esc>.

******************************************************/
#class auto


/*****************************************************
     The input and output buffers sizes are defined here. If these
     are not defined to be (2^n)-1, where n = 1...15, or they are
     not defined at all, they will default to 31 and a compiler
     warning will be displayed.
******************************************************/
#define BINBUFSIZE  15
#define BOUTBUFSIZE 15

// This timeout period determines when an active input data stream is
//   considered to have ended and is implemented within serBread.
#define MSSG_TMOUT 3000UL  // will discontinue collecting data 3 seconds after
                           // receiving any character or when maxSize are read.
                          
// This timeout period determines when to give up waiting for any data to
//   arrive and must be implemented within the user's program, as it is below.                          
#define IDLE_TMOUT 20000UL // will timeout after 20 seconds if no data is read.

typedef struct {
	short d;
	float f[20];
} DATA;    // cof_serBread and cof_serBwrite read and write bytes of any data type           

void main()
{
	unsigned long t;
	static const char Esc = 27;
	int n, maxs;
	DATA data;
	char s[32];

	// This is necessary for initializing RS232 functionality of LP35XX boards.
#if _BOARD_TYPE_ == 0x1200 || _BOARD_TYPE_ == 0x1201
	brdInit();
#endif
	
   maxs = sizeof(DATA);   // 82 bytes
	n = 0;
   serBopen(19200);
   for (;;) {
		loophead();
   	costate {
   		t = MS_TIMER;
   		wfd n = cof_serBread(&data, maxs, MSSG_TMOUT); // yields until ends
   		if (n == 1 && (char) data.d == Esc) {          // quit if just <Esc>
	      	wfd cof_serBputs("\r\nEscaped!\r\n");
	      	break;
   		}
   		else {
	      	wfd cof_serBwrite(&data, n);                   
	   		if (n != maxs) {
	   			sprintf (s, "\r\n%d characters read.\r\n", n);
	     			wfd cof_serBputs(s);
     			}
      	}
      }
      costate {
    		if (MS_TIMER > t + IDLE_TMOUT)
    		{
    			t = MS_TIMER; 
    			wfd cof_serBputs("\r\nTimed out!\r\n");
    		}  	
      }
   }

   while (serBwrFree() != BOUTBUFSIZE);	// wait for Tx buffer empty
   serBclose();									// disables B serial port r/w
}
