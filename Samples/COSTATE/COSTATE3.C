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

     costate3.c

     Advanced example of costatement use.
	  This demonstrates the difference between init_on,
	  always_on, and unnamed costatements. It also
	  demonstrates the use of CoBegin, CoResume, and
	  CoPause.
******************************************************/
#class auto

CoData cd1,cd2,cd3;

main(){

	// This so that cd3 executes once
	CoBegin(&cd3);

	while(1){

		costate cd1 init_on
		{
			// Pause cd2 every 2 seconds
			waitfor(DelaySec(2));	
			printf("pausing cd2\n");
			CoPause(&cd2);

			// Resume cd2 for 2 seconds
			waitfor(DelaySec(2));	
			printf("resuming cd2\n");
			CoResume(&cd2);

			// This is needed to reenter cd1 because is
			// not always_on.
			CoBegin(&cd1);
		}

		// This doesn't require a CoBegin to run, but it could 
		// still be paused with CoPause(&cd2);
		costate cd2 always_on
		{
			waitfor(DelayMs(200));
			printf("in cd2\n");
		}

		// If a named costatement is declared neither always_on,
		// nor init_on, it will be initialized as a PAUSED init_on
		// costatement.
		// This will only execute once unless CoBegin(cd3)
		// is uncommented out
		costate cd3
		{
			printf("in cd3\n");
			waitfor(DelayMs(400));
//			CoBegin(&cd3);
		}

		// This unnamed costatement will be entered everytime
		// through the loop and can never be paused
		costate 
		{
			printf("in unnamed costate\n");
			waitfor(DelaySec(5));
		}
		
	}
}

