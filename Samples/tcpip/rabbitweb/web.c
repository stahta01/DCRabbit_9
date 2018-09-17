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
        Samples\TcpIp\RabbitWeb\web.c

        Demonstrates the basic use of the #web registration statement in the 
        RabbitWeb HTTP enhancements.  This statement registers global variables
        in the C program with the web server such that ZHTML scripting tags can
        be used to access them.  See also the

        samples\tcpip\rabbitweb\pages\web.zhtml

        page that demonstrates some corresponding ZHTML scripting features for
        displaying and accessing the variables.

*******************************************************************************/

/***********************************
 * Configuration                   *
 * -------------                   *
 * All fields in this section must *
 * be altered to match your local  *
 * network settings.               *
 ***********************************/

/*
 * Pick the predefined TCP/IP configuration for this sample.  See
 * LIB\TCPIP\TCP_CONFIG.LIB for instructions on how to set the
 * configuration.
 */
#define TCPCONFIG 1

/********************************
 * End of configuration section *
 ********************************/

/*
 * This is needed to be able to use the RabbitWeb HTTP enhancements and the
 * ZHTML scripting language.
 */
#define USE_RABBITWEB 1

#memmap xmem

#use "dcrtcp.lib"
#use "http.lib"

/*
 * This page contains the ZHTML portion of the demonstration
 */
#ximport "samples/tcpip/rabbitweb/pages/web.zhtml"	web_zhtml

/* The default mime type for '/' must be first */
SSPEC_MIMETABLE_START
   // This handler enables the ZHTML parser to be used on ZHTML files...
	SSPEC_MIME_FUNC(".zhtml", "text/html", zhtml_handler),
	SSPEC_MIME(".html", "text/html")
SSPEC_MIMETABLE_END

/* Associate the #ximported files with the web server */
SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/", web_zhtml),
	SSPEC_RESOURCE_XMEMFILE("/index.zhtml", web_zhtml)
SSPEC_RESOURCETABLE_END

// The following global variables will be registered with the #web statement.
char my_string[21];
int inum;
long lnum;
unsigned int uinum;
char character;
float fpoint;

// This will be used to demonstrate how normal variables can be referenced in
// #web guard expressions.
const float pi = 3.14159;

// This function will be used to check the value of uinum.
int check_uinum(unsigned int num);

/*
 * #web statements
 */
// This shows a simple registration of a string without any guard expression
// to check the value.  Note, however, that the length of strings will be
// automatically checked so that they do not overflow the buffer.
#web my_string
// This registers the integer inum with a guard expression.  Note that the
// '$inum' refers to the *new* value of inum.  If simply (inum > 100) were
// given, the guard would check the old, committed value of inum against 100,
// not the new version given by the user in the web interface.
#web inum ($inum > 100)
// Note that this guard checks $lnum twice to put the value in a range.
#web lnum (($lnum > 50000) && ($lnum < 100000))
// uinum is checked by a separate function, which ensures that it is divisible
// by 3.
#web uinum (check_uinum($uinum))
// character must be a capital letter.
#web character (($character >= 'A') && ($character <= 'Z'))
// fpoint must be between pi and 2*pi.  Note that you can refer to normal C
// variables within guard expressions, including #web-registered variables.
#web fpoint (($fpoint >= pi) && ($fpoint <= 2*pi))

// Check the uinum value.
int check_uinum(unsigned int num)
{
	return ((num % 3) == 0);
}

void main(void)
{
	// Give sane defaults to the global variables
	strcpy(my_string, "Hello, world!");
	inum = 150;
	lnum = 60000;
	uinum = 123;
	character = 'Z';
	fpoint = 4.0;

	// Initialize the TCP/IP stack and HTTP server
   sock_init();
   http_init();

	// This yields a performance improvement for an HTTP server
	tcp_reserveport(80);

   while (1) {
		// Drive the HTTP server
      http_handler();
   }
}