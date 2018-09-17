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
/*******************************************************************************

	nflash_inspect.c

	This program is used with BL2600 or RCM3300 series controllers equipped with
	a supported NAND flash device and/or an xD Picture Card containing a
   supported NAND flash device.


	Description
	===========
	This program is a handy utility for inspecting the contents of a NAND flash
	device.  When it starts it attempts to initialize the user selected NAND
	flash device.  If initialization is successful, the user is presented with
	the following menu of options:
		'C' clears (to zero) all bytes in specified page;
		'D' writes xD_page data into absolute page 0x20;
		'E' erases (to 0xFF) all bytes in specified erase block;
		'P' prints out contents of specified page;
		'Q' quits program (automatic restart in run mode);
		'W' writes specified value to all bytes in specified page;
		'X' writes count pattern into specified page.

	The user should be aware of the trade-offs made by this sample in defining
	the NFLASH_USEERASEBLOCKSIZE macro value to be 0 (zero); this mode makes the
	nand flash driver use smaller (512 byte) chunks of data, which are less
	tedious to manage in this program than using the alternative larger (16 KB)
	chunks of data.  However, using smaller chunks of data means more nand flash
	block erases are required to update all of the program pages in an erase
	block, one per program page written.  In contrast, updating all of the
	program pages in an erase block can require only a single block erase when
	all of the program pages within the erase block are treated as a single large
	page.  See the nf_initDevice() function's help for more information.


	Instructions
	============
	1. Compile and run this program.
	2. Follow the prompts.

*******************************************************************************/

// These defines redirect run mode STDIO to serial port A at 57600 baud.
#define STDIO_DEBUG_SERIAL SADR
#define STDIO_DEBUG_BAUD 57600

// This define adds carriage returns ('\r') to each newline char ('\n') when
//  sending STDIO output to a serial port.
#define STDIO_DEBUG_ADDCR

// Uncomment this define to force both run mode and debug mode STDIO to the
//  serial port specified above.
//#define STDIO_DEBUG_FORCEDSERIAL

#class auto
#if (_BOARD_TYPE_ & 0xFF00) == (RCM3360A & 0xFF00)
#use "rcm33xx.lib"	// sample library to use with this application
#endif

// Caution:  If NFLASH_CANERASEBADBLOCKS is defined before NFLASH.LIB is used,
//           then the nand flash driver will allow bad blocks to be erased,
//           including the bad block marker itself.  Thereafter, data that is
//           stored in the unmarked bad block may not be recoverable.
//#define NFLASH_CANERASEBADBLOCKS

// if NFLASH_USEERASEBLOCKSIZE is not defined, its value defaults to 1
//  0 == use 512 byte main data page size; 1 == use 16 KB main data page size
#define NFLASH_USEERASEBLOCKSIZE 0

//#define NFLASH_VERBOSE
//#define NFLASH_DEBUG
#use "nflash.lib"	// base nand flash driver library

// local function prototypes
long input_number(void);

// global constants

// this page data is important for xD Cards to be recognized by a PC reader
const char xD_page[512] = {
0x01,0x03,0xd9,0x01,0xff,0x18,0x02,0xdf,0x01,0x20,0x04,0x00,0x00,0x00,0x00,0x21,
0x02,0x04,0x01,0x22,0x02,0x01,0x01,0x22,0x03,0x02,0x04,0x07,0x1a,0x05,0x01,0x03,
0x00,0x02,0x0f,0x1b,0x08,0xc0,0xc0,0xa1,0x01,0x55,0x08,0x00,0x20,0x1b,0x0a,0xc1,
0x41,0x99,0x01,0x55,0x64,0xf0,0xff,0xff,0x20,0x1b,0x0c,0x82,0x41,0x18,0xea,0x61,
0xf0,0x01,0x07,0xf6,0x03,0x01,0xee,0x1b,0x0c,0x83,0x41,0x18,0xea,0x61,0x70,0x01,
0x07,0x76,0x03,0x01,0xee,0x15,0x14,0x05,0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
0x00,0x20,0x20,0x20,0x20,0x00,0x31,0x2e,0x30,0x00,0xff,0x14,0x00,0xff,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x01,0x03,0xd9,0x01,0xff,0x18,0x02,0xdf,0x01,0x20,0x04,0x00,0x00,0x00,0x00,0x21,
0x02,0x04,0x01,0x22,0x02,0x01,0x01,0x22,0x03,0x02,0x04,0x07,0x1a,0x05,0x01,0x03,
0x00,0x02,0x0f,0x1b,0x08,0xc0,0xc0,0xa1,0x01,0x55,0x08,0x00,0x20,0x1b,0x0a,0xc1,
0x41,0x99,0x01,0x55,0x64,0xf0,0xff,0xff,0x20,0x1b,0x0c,0x82,0x41,0x18,0xea,0x61,
0xf0,0x01,0x07,0xf6,0x03,0x01,0xee,0x1b,0x0c,0x83,0x41,0x18,0xea,0x61,0x70,0x01,
0x07,0x76,0x03,0x01,0xee,0x15,0x14,0x05,0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
0x00,0x20,0x20,0x20,0x20,0x00,0x31,0x2e,0x30,0x00,0xff,0x14,0x00,0xff,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

// global variables

// protected in separate battery backed /CS2 SRAM, when one is available
protected nf_device nandFlash;
char hexstr[8];


int main()
{
	auto char inchar, pageval;
	auto int status;
	auto long bufAddr, bufSize, myMainBuffer, pagenum, endnum;

	_sysIsSoftReset();	// restore any protected variables

	brdInit();
   *hexstr = 0;         // default to decimal entry

	do {
	   printf("Which NAND flash device? (0 == soldered, 1 == socketed):  ");
	   inchar = getchar();
	   printf("%c\n", inchar);
   	switch (inchar) {
   	case '0':
		case '1':
			break;
		default:
			printf("Unknown selection, try again.\n");
   	}
   } while (('0' != inchar) && ('1' != inchar));

	if (nf_initDevice(&nandFlash, (int) (inchar - '0'))) {
   	printf("\nNAND flash init failed!\n");
      exit(-1);
   } else {
   	printf("\nNAND flash init OK.\n");
      printf("# of blocks: 0x%08LX\n", nf_getPageCount(&nandFlash));
      printf("size of block: 0x%08LX\n", nf_getPageSize(&nandFlash));
   }

  	// Use a temp variable here, to avoid the possibility of _xalloc attempting
  	//  to change nf_getPageSize(&nandFlash)'s value!
  	bufSize = nf_getPageSize(&nandFlash);
  	// Get application's main data buffer.  (Note that _xalloc will cause a run
  	//  time error if there is not sufficient xmem data RAM available.)
#if BBROOTDATASIZE
	// If available, use the separate battery backed SRAM for the main data
	//  buffer.  This allows the nand flash driver to possibly recover a write
	//  interrupted by a power cycle.
	myMainBuffer = _xalloc(&bufSize, 0, XALLOC_BB);
#else
	myMainBuffer = _xalloc(&bufSize, 0, XALLOC_ANY);
#endif

   while (1) {
   	printf("\nChoose one of the following options:\n");
		printf("   'C' clears (to zero) all bytes in specified page;\n");
		printf("   'D' writes xD_page data into absolute page 0x20;\n");
		printf("   'E' erases (to 0xFF) all bytes in specified erase block;\n");
		printf("   'H' toggles hex input mode for numerical entry;\n");
		printf("   'P' prints out contents of specified page;\n");
		printf("   'R' prints out contents of a range of pages;\n");
		printf("   'Q' quits program (automatic restart in run mode);\n");
		printf("   'W' writes specified value to all bytes in specified page;\n");
		printf("   'X' writes count pattern into specified page.\n");
   	inchar = getchar();
   	printf("\n");

		switch (tolower(inchar)) {
		case 'c':
			printf("Page number %sto clear (all bytes to zeros)?  ", hexstr);
			pagenum = input_number();
			printf("\n");
			if ((pagenum >= 0L) && (pagenum < nf_getPageCount(&nandFlash))) {
				pageval = 0;
				bufAddr = myMainBuffer + nf_getPageSize(&nandFlash);
				while (bufAddr > myMainBuffer) {
					--bufAddr;
					root2xmem(bufAddr, &pageval, sizeof(char));
				}
				status = nf_writePage(&nandFlash, myMainBuffer, pagenum);
				if (status) {
					printf("\nError, nf_writePage() result was %d!\n", status);
				}
			} else {
				printf("\nError, invalid page number.\n");
			}
			break;
		case 'd':
#if NFLASH_USEERASEBLOCKSIZE
			if (0x01L < nandFlash.startpage) {
				printf("Can't write 'xD_page' data to absolute page 0x20.\n");
				printf("Try recompiling with NFLASH_CANERASEBADBLOCKS defined.\n");
				status = 0;
			} else {
				status = nf_readPage(&nandFlash, myMainBuffer, 0x01L);
				if (status) {
					printf("Error pre-reading 'xD_page' data page's erase block!\n");
					status = 0;
				} else {
					root2xmem(myMainBuffer, xD_page, sizeof(xD_page));
					status = nf_writePage(&nandFlash, myMainBuffer,
					                      0x01L - nandFlash.startpage);
				}
			}
#else
			if (0x20L < nandFlash.startpage) {
				printf("Can't write 'xD_page' data to absolute page 0x20.\n");
				printf("Try recompiling with NFLASH_CANERASEBADBLOCKS defined.\n");
				status = 0;
			} else {
				status = nf_writePage(&nandFlash, paddr(xD_page),
				                      0x20L - nandFlash.startpage);
			}
#endif
			if (status) {
				printf("\nError, nf_writePage() result was %d!\n", status);
			}
			break;
		case 'e':
			printf("Page number %sto erase (entire erase block to 0xFFs)?  ", hexstr);
			pagenum = input_number();
			printf("\n");
			if ((pagenum >= 0L) && (pagenum < nf_getPageCount(&nandFlash))) {
				status = nf_eraseBlock(&nandFlash, pagenum);
				if (status) {
					printf("\nError, nf_erasePage() result was %d!\n", status);
				}
			} else {
				printf("\nError, invalid page number.\n");
			}
			break;
		case 'h':
      	if (*hexstr) {
         	printf("\nDecimal input mode set.\n");
         	*hexstr = 0;
         }
         else {
         	printf("\nHexadecimal input mode set.\n");
         	strcpy(hexstr, "in hex ");
         }
         break;
		case 'p':
			printf("Page number %sto print out?  ", hexstr);
			pagenum = input_number();
			printf("\n");
			if ((pagenum >= 0L) && (pagenum < nf_getPageCount(&nandFlash))) {
				status = nf_readPage(&nandFlash, myMainBuffer, pagenum);
				if (status) {
					printf("\nError, nf_readPage() result was %d!\n", status);
				}
				_nf_print_data("\nApplication's main", pagenum, myMainBuffer,
				              nf_getPageSize(&nandFlash));
				printf("\n");
			} else {
				printf("\nError, invalid page number.\n");
			}
			break;
		case 'r':
			printf("Beginning page number %sto print out?  ", hexstr);
			pagenum = input_number();
			printf("\n");
			if ((pagenum < 0L) || (pagenum > nf_getPageCount(&nandFlash))) {
				printf("\nError, invalid page number.\n");
            break;
         }
			printf("Ending page number %sto print out?  ", hexstr);
			endnum = input_number();
			printf("\n");
			if ((endnum < 0L) || (endnum > nf_getPageCount(&nandFlash))) {
				printf("\nError, invalid page number.\n");
            break;
         }
			while (pagenum <= endnum) {
				status = nf_readPage(&nandFlash, myMainBuffer, pagenum);
				if (status) {
					printf("\nError, nf_readPage() result was %d!\n", status);
				}
				_nf_print_data("\nApplication's main", pagenum++, myMainBuffer,
				              nf_getPageSize(&nandFlash));
				printf("\n");
			}
			break;
		case 'q':
			goto end_program;
			break;
		case 'w':
			printf("Page number %sto write (all bytes to specified value)?  ", hexstr);
			pagenum = input_number();
			printf("\n");
			if ((pagenum >= 0L) && (pagenum < nf_getPageCount(&nandFlash))) {
         	if (*hexstr)
            	printf("Hexadecimal byte value to write? ");
            else
					printf("Decimal byte value to write?  ");
				pageval = (char) input_number();
				printf("\n");
				bufAddr = myMainBuffer + nf_getPageSize(&nandFlash);
				while (bufAddr > myMainBuffer) {
					--bufAddr;
					root2xmem(bufAddr, &pageval, sizeof(char));
				}
				status = nf_writePage(&nandFlash, myMainBuffer, pagenum);
				if (status) {
					printf("\nError, nf_writePage() result was %d!\n", status);
				}
			} else {
				printf("\nError, invalid page number.\n");
			}
			break;
		case 'x':
			printf("Page number %sto write (count pattern into)?  ", hexstr);
			pagenum = input_number();
			printf("\n");
			if ((pagenum >= 0L) && (pagenum < nf_getPageCount(&nandFlash))) {
				// into each main data buffer byte, put its physical address LSB
				bufAddr = myMainBuffer + nf_getPageSize(&nandFlash);
				while (bufAddr > myMainBuffer) {
					--bufAddr;
					root2xmem(bufAddr, &bufAddr, sizeof(char));
				}
				// put page number marker into each end of the main data buffer
				xsetlong(myMainBuffer, pagenum);
				xsetlong(myMainBuffer + nf_getPageSize(&nandFlash) -
				         (long) sizeof(long), pagenum);
				status = nf_writePage(&nandFlash, myMainBuffer, pagenum);
				if (status) {
					printf("\nError, nf_writePage() result was %d!\n", status);
				}
			} else {
				printf("\nError, invalid page number.\n");
			}
			break;
		default:
			printf("\nUnknown option, try again.\n");
		}	// end of switch
	}	// end of while

// Come here to end the program, so that it may be restarted manually (debug
//  mode) or will be restarted automatically (run mode).
end_program:
}


// Gets positive numeric input from keyboard and returns long value when enter
// key is pressed.  Returns -1 if non-numeric keys are pressed.  (Allows
// backspace.)
long input_number()
{
	auto long number;
   auto char inchar;

   number = 0L;
   while (1) {
   	inchar = toupper(getchar());
      putchar(inchar); //echo input
      if (inchar == '\n' || inchar == '\r') {
      	return number;
      }
      else if (inchar == '\b') {
         //backspace
         if (*hexstr)
         	number >>= 4;
         else
	      	number = number / 10L;
      }
      else {
         if (*hexstr)
      		number <<= 4;
         else
	      	number *= 10L;
	      if (inchar >= '0' && inchar <= '9') {
   	   	number += inchar - '0';
      	} else {
         	if (*hexstr) {
            	if (inchar >= 'A' && inchar <= 'F') {
               	number += inchar - 55;
                  continue;
               }
            }
      		//bad input
	         return -1L;
   	   }
      }
   }	// end of while
}

