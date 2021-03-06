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
/***************************************************************************
	ad_rdvolt_ch.c

	This sample program is used with LP3500 series controllers.

	Description
	===========
	This program reads and displays voltage and equivalent values of one
	single-ended ADC channel.  Coefficients are read from the simulated
	EEPROM (Flash)	to compute equivalent voltages.
	Computed raw data and equivalent voltages will be displayed.

	Instructions
	============
	1. Connect a power supply of 0-20 volts to an input channel.
	2. Compile and run this program.
	3. Follow the prompted directions of this program during execution.
	4. Values will continuously display.

***************************************************************************/
#class auto					/* Change local var storage default to "auto" */

#define STARTCHAN	0
#define ENDCHAN 7
#define GAINSET GAIN_1		//other gain macros
									//GAIN_1
									//GAIN_2
									//GAIN_4
									//GAIN_5
									//GAIN_8
									//GAIN_10
									//GAIN_16
									//GAIN_20

//---------------------------------------------------------
//	displays both the raw data count and voltage equivalent
//	_adsCalibS is address for single ended channels
//---------------------------------------------------------
void anaInInfo (unsigned int channel, unsigned int *rd, float *ve)
{
	auto unsigned value;
	auto float volt;

	value = anaIn(channel, SINGLE, GAINSET);
	volt = (float)((int)(value - _adcCalibS[channel][GAINSET].offset)*(_adcCalibS[channel][GAINSET].kconst));

	if (value == ADOVERFLOW)
	{
		*rd = ADOVERFLOW;
		*ve = ADOVERFLOW;
	}
	else
	{
		*rd = value;
		if (value <= 0.00)
			*ve = 0.000;
		else
			*ve = volt;
	}
}


void main()
{
	auto unsigned int rawdata;
	auto int inputnum, keypress;
	auto float voltequ;
	auto char buffer[64];

	brdInit();			//read constants if AD device installed

	while (1)
	{
		printf("\nChoose the AD channel %d to %d .... ", STARTCHAN, ENDCHAN);
		gets(buffer);
		inputnum = atoi(buffer);

		printf("\nChoose:\n");
		printf("  1 to display raw data only\n");
		printf("  2 to display voltage only\n");
		printf("  3 to display both\n  .... ");
		gets(buffer);
		keypress = atoi(buffer);

		while (strcmp(buffer,"q") && strcmp(buffer,"Q"))
		{
			switch (keypress)
			{
				case 1:
					rawdata = anaIn(inputnum, SINGLE, GAINSET);
					printf("CH%2d raw data %d\n", inputnum, rawdata);
					break;
				case 2:
					voltequ = anaInVolts(inputnum, GAINSET);
					printf("CH%2d is %.5f V\n", inputnum, voltequ);
					break;
				case 3:
					anaInInfo(inputnum, &rawdata, &voltequ);
					printf("CH%2d is %.5f V from raw data %d\n", inputnum, voltequ, rawdata);
					break;
				default:
				break;
			}
			printf("\npress enter key to read value again or 'Q' or read another channel\n\n");
			gets(buffer);
		}
	}
}	//end main

