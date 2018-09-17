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
/********************************************************
		ICOFUNC.C

		This example demonstrates how to use indexed
		cofunctions. Note the "[4]" in the definition of
		the cofunction, and array-style subscripts that
		are used when it is invoked. 
*********************************************************/
#class auto

/******************************************************/
/* The worker function. This will be entered multiple */
/* times from main(), creating four seperate tasks    */
/* that run simultainously.                           */
/******************************************************/
cofunc int worker[4](char *name)
{
	auto int temp; // variables should be 'auto',
						// so you get multiple copies!
	static int count; // static, so it is shared by all workers

	// start our count off at 0, once at the very
	// beginning of our program.
#GLOBAL_INIT {
	count = 0; 
}
							
	// do some work - just increment a global counter
	temp = ++count;

	// print the results
	printf("worker \"%s\" running, %d tasks have run\n", name, temp);
	
	// return true
	return 1;
}

/*********************************/
/* Main Program loop starts here */
/*********************************/
main()
{
 	while (1) {
   	costate {
         wfd{ worker[0]("A"); worker[1]("B"); worker[2]("C"); worker[3]("D"); }
      }
      costate {
         // Other code can be inserted here, to run
         // at the same time as the above worker tasks
        	waitfor(DelayMs(100));
		}    
  	}
}

