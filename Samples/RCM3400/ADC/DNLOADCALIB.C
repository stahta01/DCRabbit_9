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

	dnloadcalib.c

	This sample program is for the RCM3400 series controllers with
	prototyping boards.

	This program demonstrates how to retrieve your analog calibration data
 	to rewrite it back to simulated eeprom in flash with using a serial
 	utility such as Tera Term.

	Description
	===========
	This program demonstrates the sending of a data file to download
	calibrations constants to a controller's user block in Flash and
	by transmitting the file using a	serial port a PC serial utility
	such as Tera Term.

	Note: To upload calibrations to be used by this program, use
			uploadcalib.c

	!!!This program must be compiled to Flash.

	The Tera Term serial utility can be downloaded from their WEB page
	located at:
	http://hp.vector.co.jp/authors/VA002416/teraterm.html

  	Hardware setup
  	==============
	1. Connect PC (tx) to the controller RXC J5.
  	2. Connect PC (rx) to the controllerTXC on J5.
	3. Connect PC GND to GND on J4.

	Tera Term setup
	===============
	1. Startup Tera Term on your PC.
	2. Configure the serial parameters for the following:
	   a) Baud Rate of 19200, 8 bits, no parity and 1 stop bit.
	   b) Enable the "Local Echo" option.
	   c) Set line feed options to:  Receive = CR     Transmit = CR+LF

	Program Instructions
   ====================
   1. Compile and run this program. Verify that the message "Waiting,
      Please Send Data file" message is being display in Tera Term
      display window before proceeding.

   2. From within Tera Term do the following:
   	- Select...File-->Send File-->Path and filename
      a) Select the OPEN option within dialog box.

	3. Once the data file has been downloaded it will indicate if the
		calibration data was successfully written.

	Example dat file
	================

	::
	SN9MN234
	ADSE
	0
	float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,
	float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,
	1
	float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,
	float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,
		|
		|

	ADDF
	0
	float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,
	float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,
	2
	float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,
	float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,
		|
		|

	ADMA
	3
	float_gain,float_offset,
	4
	float_gain,float_offset,
		|
		|

	END
	::
	End of table upload

**************************************************************************/
#class auto

#define ADC_SCLKBAUD 115200ul
/////
// configure your serial port connection here
//	presently configured serial port C
///
#define seropen	serCopen
#define serclose	serCclose
#define serputs	serCputs
#define serwrite	serCwrite
#define serread	serCread
#define sergetc	serCgetc
#define serrdFlush serCrdFlush
#define serwrFlush serCwrFlush
#define serrdFree	serCrdFree
#define serwrFree	serCwrFree
#define SXSR SCSR
#define CINBUFSIZE 255
#define COUTBUFSIZE 255

#define INBUFSIZE CINBUFSIZE
#define OUTBUFSIZE COUTBUFSIZE
#define BAUDRATE 19200l

#define FILEBUFSIZE	4096	//4K max file size
#define TERMINATOR	'\n'

char string[128];
char buffer[128];

nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long done_time;

	done_time = MS_TIMER + delay;
   while( (long) (MS_TIMER - done_time) < 0 );
}

/////////////////////////////////////////////////////////////////////
// Calibration data error handler
/////////////////////////////////////////////////////////////////////
void caldata_error(char *ptr, char *msg)
{
	memset(ptr, 0x20, 80);
	ptr[0]  = '\r';
	ptr[80] = '\0';
	serwrite(ptr, strlen(ptr));
	sprintf(ptr, msg);
	serwrite(ptr, strlen(ptr));

	// Make sure all data gets transmitted before exiting the program
	while (serwrFree() != OUTBUFSIZE);
   while((RdPortI(SXSR)&0x08) || (RdPortI(SXSR)&0x04));
}


/////////////////////////////////////////////////////////////////////
// Locate the calibration data within the file using known
// identifier TAGS.
/////////////////////////////////////////////////////////////////////
unsigned long find_tag(unsigned long fileptr, long len)
{
	auto char data[2047];
	auto long index,i;
	auto char *begptr, *firstline, *secondline, *saveptr;
	auto int eofile, eoline, nextcase, dnstate, channel, gaincode;

	index = 0;
	xmem2root(data, fileptr, (int)len);
	begptr = strtok(data, "\n\r");		//begin data file
	while (strncmp(begptr, "::", 2))		//look for start
	{
		begptr = strtok(NULL, "\n\r");
	}
	begptr = strtok(NULL, "\n\r");
	serputs("\n\rData file serial number is \x0");
	serwrite(begptr, strlen(begptr));

	eofile = FALSE;
	saveptr = begptr+strlen(begptr)+1;
	while (!eofile)
	{
		eoline = FALSE;
		nextcase = 0;
		begptr = strtok(saveptr, "\n\r");
		saveptr = begptr+strlen(begptr)+1;
		if (!strncmp(begptr, "ADSE", 4))
		{
			dnstate = 2;
		}
		else
			if (!strncmp(begptr, "ADDF", 4))
			{
				dnstate = 3;
			}
			else
				if (!strncmp(begptr, "ADMA", 4))
				{
					dnstate = 4;
				}
				else
					if (!strncmp(begptr, "END", 3))
					{
						eofile = TRUE;
						eoline = TRUE;
					}
					else
						nextcase = 1;

		while (!eoline)
		{
			switch (nextcase)
			{
				case 2:		//single ended
					firstline = strtok(NULL, "\n\r");
					secondline = strtok(NULL, "\n\r");
					for (gaincode = 0; gaincode <= 3; gaincode++)
					{
						begptr = strtok(firstline, ",");
						_adcCalibS[channel][gaincode].kconst = atof(begptr);
						begptr = strtok(NULL, ",");
						_adcCalibS[channel][gaincode].offset = atof(begptr);
						firstline = begptr+strlen(begptr)+1;
					}

					for (gaincode = 4; gaincode <= 7; gaincode++)
					{
						begptr = strtok(secondline, ",");
						_adcCalibS[channel][gaincode].kconst = atof(begptr);
						begptr = strtok(NULL, ",");
						_adcCalibS[channel][gaincode].offset = atof(begptr);
						secondline = begptr+strlen(begptr)+1;
					}
					saveptr = secondline+1;
					eoline = TRUE;
					break;
				case 3:		//differential
					firstline = strtok(NULL, "\n\r");
					secondline = strtok(NULL, "\n\r");
					for (gaincode = 0; gaincode <= 3; gaincode++)
					{
						begptr = strtok(firstline, ",");
						_adcCalibD[channel][gaincode].kconst = atof(begptr);
						begptr = strtok(NULL, ",");
						_adcCalibD[channel][gaincode].offset = atof(begptr);
						firstline = begptr+strlen(begptr)+1;
					}
					for (gaincode = 4; gaincode <= 7; gaincode++)
					{
						begptr = strtok(secondline, ",");
						_adcCalibD[channel][gaincode].kconst = atof(begptr);
						begptr = strtok(NULL, ",");
						_adcCalibD[channel][gaincode].offset = atof(begptr);
						secondline = begptr+strlen(begptr)+1;
					}
					saveptr = secondline+1;
					eoline = TRUE;
					break;
				case 4:		//milli-amp
					firstline = strtok(NULL, "\n\r");
					begptr = strtok(firstline, ",");
					_adcCalibM[channel].kconst = atof(begptr);
					begptr = strtok(NULL, ",");
					_adcCalibM[channel].offset = atof(begptr);
					saveptr = begptr+strlen(begptr)+2;
					eoline = TRUE;
					break;
				case 1:
					channel = atoi(begptr);
					nextcase = dnstate;
					eoline = FALSE;
					break;
				case 0:
					eoline = TRUE;
					break;
			} //switch

		} //while not eoline
	} //while not eofile

	anaInEEWr(ALLCHAN, SINGLE, gaincode);			//read all single-ended
	anaInEEWr(ALLCHAN, DIFF, gaincode);				//read all differential
	anaInEEWr(ALLCHAN, mAMP, gaincode);				//read all milli-amp

}


/////////////////////////////////////////////////////////////////////
//	Read the file from the serial port
/////////////////////////////////////////////////////////////////////
unsigned long getfile( unsigned long xmem_ptr )
{
	auto char buffer[256];
	auto unsigned int len;
	auto unsigned int total_len, file_done;

	serrdFlush();
	while (serrdFree() == INBUFSIZE);
	total_len = 0;
	file_done = FALSE;
	while(!file_done)
	{
		// Use the serial timeout to determine that the dowload is completed
		if((len = serread(buffer, 256, 100)) < 256)
		{
			file_done = TRUE;
		}
		if(!file_done)	msDelay(100);

		// Move data to large XMEM buffer
		root2xmem((xmem_ptr+total_len), buffer, len);
		total_len += len;
	}
	return(total_len);
}


/////////////////////////////////////////////////////////////////////
//	Retrieve analog calibration data and rewrite to the flash
/////////////////////////////////////////////////////////////////////
void main()
{
	auto unsigned long fileptr, tempPtr, xmemPtr, index;
	auto unsigned long len;
	auto int i;
	auto char serialNumber[64];

	//------------------------------------------------------------------------
	//		Initialize the Controller
	//------------------------------------------------------------------------
	brdInit();
	seropen(BAUDRATE);	//set baud rates for the serial ports to be used
	serwrFlush();		//clear Rx and Tx data buffers
	serrdFlush();

	//------------------------------------------------------------------------
	//		Allocate and Clear XMEM
	//------------------------------------------------------------------------

	// Allocate XMEM memory for the file that will be read in from the PC
	xmemPtr = xalloc(FILEBUFSIZE);

	// Clear the buffer in XMEM
	for(index =0; index < FILEBUFSIZE; index++)
	{
		root2xmem(xmemPtr + index, "\x00", 1);
	}

	//------------------------------------------------------------------------
	//		Download the Data File from the PC
	//------------------------------------------------------------------------
	sprintf(string, "\r\nWaiting...Please Send Data file\n\r");
	serwrite(string, strlen(string));

	// Get the calibration data file from the PC and put it into XMEM
	if(!(len = getfile(xmemPtr)))
	{
		caldata_error(string, "\r\n\nEncounter an error while reading calibration file");
		exit(1);
	}
	fileptr = xmemPtr;
	sprintf(string, "\r\n\nDownload Complete\n\n\r");
	serwrite(string, strlen(string));

	//------------------------------------------------------------------------
	//	 Parse data file and write to calibrations to flash
	//------------------------------------------------------------------------
	sprintf(string, "\r\nParsing data file\n\r");
	serwrite(string, strlen(string));

	tempPtr = find_tag(fileptr, len);

	sprintf(string, "\r\n\nExiting....Calibration data successfully written\n\n\r");
	serwrite(string, strlen(string));
	while (serwrFree() != OUTBUFSIZE);
   while((RdPortI(SXSR)&0x08) || (RdPortI(SXSR)&0x04));
	serclose();
}