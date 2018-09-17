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
        Samples\TcpIp\RabbitWeb\selection.c

        Demonstrates the use of the selection variables feature in the 
        RabbitWeb HTTP enhancements.  See also the

        samples\tcpip\rabbitweb\pages\selection.zhtml

        page that demonstrates the corresponding ZHTML scripting features for
        selection variables.

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
 * This page contains the ZHTML portion of the selection variable demonstration
 */
#ximport "samples/tcpip/rabbitweb/pages/selection.zhtml"	selection_zhtml

/* The default mime type for '/' must be first */
SSPEC_MIMETABLE_START
   // This handler enables the ZHTML parser to be used on ZHTML files...
	SSPEC_MIME_FUNC(".zhtml", "text/html", zhtml_handler),
	SSPEC_MIME(".html", "text/html")
SSPEC_MIMETABLE_END

/* Associate the #ximported files with the web server */
SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/", selection_zhtml),
	SSPEC_RESOURCE_XMEMFILE("/index.zhtml", selection_zhtml)
SSPEC_RESOURCETABLE_END

/*
 * Selection variables to be registered.  Note that you can use watch
 * expressions or the evaluate expression feature during runtime to ensure that
 * the variables have properly changed values.
 */

// This is an integer selection variable.
int color;
// This is a long selection variable.  Note that unsigned ints and longs can
// also be used.
long city;

/*
 * #web statements
 */

// The following statement registers the global variable color as a selection
// variable.  Note that '"Blue" = 1' indicates that if, in the HTML interface,
// the selection "Blue" is made, then the variable "color" will have value 1.
// The select() registration statement works much like an enum declaration.
// "Green" will be 2, "Yellow" will be 3, etc.
#web color select("Blue" = 1, "Green", "Yellow", "Orange", "Red", "Purple")

// The following registers city.  Note that each option in the select statement
// is given an explicit value, since each city should correspond to its ZIP
// code.
#web city select("Davis, CA" = 95616, "Leland, MS" = 38756, \
                 "Jefferson, WI" = 53549)

void main(void)
{
	// Initialize the global, #web-registered variables.
	color = 3;
	city = 53549;

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

