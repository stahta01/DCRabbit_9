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
        Samples\TcpIP\TFTP\tftpclnt.c

        Demonstrate how to use TFTP (Trivial File Transfer Protocol) stand-alone
        i.e. without also using BOOTP/DHCP.

        To run this demo, you need to fill in the address of a TFTP server
        machine, as well as the name of a file to download.  The server will
        need to be set up to allow access to the specified file names.
        TFTP servers are finicky about permissions, since there is no
        security with TFTP.  Make sure all files exist and are given world-
        readable/writable permissions.

*******************************************************************************/
#class auto

#define MAX_UDP_SOCKET_BUFFERS 2
//#define TFTP_ALLOW_BUG	// work-around RH7.0 bug

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

/*
 * Define TFTP server, download and upload file names, and max download length.
 */
#define TFTP_SERVER		"10.10.6.111"
#define TFTP_DL_FILENAME	"/tftpboot/foo"
#define TFTP_UL_FILENAME	"/tftpboot/bar"
#define TFTP_DL_SIZE		3000

#memmap xmem
#use "dcrtcp.lib"
#use "tftp.lib"


udp_Socket tsock;		// Define UDP socket for TFTP use

int main()
{
	struct tftp_state ts;
	int status;
	word bflen;
	
   status = sock_init();
   if (status) {
   		printf("Could not init packet driver.\n");
   		exit(3);
   }
	// Wait for the interface to come up
	while (ifpending(IF_DEFAULT) == IF_COMING_UP) {
		tcp_tick(NULL);
	}
   

   /*########## DOWNLOAD ##########*/   

  	ts.state = 0;								// 0 = read
  	ts.buf_len = TFTP_DL_SIZE;				// max length to download
  	ts.buf_addr = xalloc(TFTP_DL_SIZE);	// allocate a buffer
  	ts.my_tid = 0;								// zero to use default TFTP UDP port number
  	ts.sock = &tsock;							// point to socket to use
  	ts.rem_ip = resolve(TFTP_SERVER);	// resolve server IP address
  	ts.mode = TFTP_MODE_OCTET;				// send/receive binary data
  	strcpy(ts.file, TFTP_DL_FILENAME);	// set file name on server
  	
  	printf("Downloading %s...\n", ts.file);
      	
  	// This uses the non-blocking TFTP functions, but in a blocking
  	// manner.  It would be easier to use tftp_exec(), but this
  	// doesn't return the server error message.
  	tftp_init(&ts);
	while ((status = tftp_tick(&ts)) > 0);	// Loop until complete
	if (!status)
		printf("Download completed\n");
	else if (status == -3)
		printf("ERROR: Download timed out.\n");
	else if (status == -5)
		printf("Download completed, but truncated\n");
	else {
		printf("Download failed: code %d\n", status);
		if (status == -1)
			printf("  Message from server: %s\n", ts.file);
	}


   /*########## UPLOAD ##########*/   

  	ts.state = 1;								// 0 = write
  	ts.buf_len = ts.buf_used;				// length to upload (use same buffer as downloaded)
  	ts.my_tid = 0;								// zero to use default TFTP UDP port number
  	ts.sock = &tsock;							// point to socket to use
  	ts.rem_ip = resolve(TFTP_SERVER);	// resolve server IP address
  	ts.mode = TFTP_MODE_OCTET;				// send/receive binary data
  	strcpy(ts.file, TFTP_UL_FILENAME);	// set file name on server
  	
  	printf("Uploading as %s...\n", ts.file);
      	
  	tftp_init(&ts);
	while ((status = tftp_tick(&ts)) > 0);	// Loop until complete
	if (!status)
		printf("Upload completed\n");
	else {
		printf("Upload failed: code %d\n", status);
		if (status == -1)
			printf("  Message from server: %s\n", ts.file);
		else if (status == -3)
			printf("ERROR: Download timed out.\n");
		else if (status == -2) {
			printf("  Server did not ack last packet...\n");
			printf("    Acked %u\n", ts.buf_used);
			if (ts.buf_len - ts.buf_used <= 512)
				printf("    Some bug-ridden servers don't ack the last packet sent :-(\n");
		}
	}

	printf("All done.\n");
	return 0;
}