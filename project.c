#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include "image.h"
#include "ppm.h"
#include "texture_synthesis.h"

int main( int argc , char *argv[] )
{

	// Check number of arguments is valid
	printf("started program \n");
	if(argc != 6) return 1;

	// Seed the random number generator so that the code produces the same results on the same input.
	srand(0);

	// Get the time at the start of execution
	clock_t start_clock = clock();
	printf("started timing \n");

	// TODO: IMPLEMENT THIS FUNCTION
	FILE *in = fopen(argv[1], "rb");
	printf("opened input file \n"); 

	FILE *out = fopen(argv[2], "wb"); 
	printf("opened output file \n");

	printf("started reading image \n");
	Image *img = ReadPPM(in); 
	
	printf("finished reading image \n");

	printf("started synthesizing image \n");
	Image *outImg = SynthesizeFromExemplar(img, atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), false); 
	printf("finished synthesizing image \n");

	int status = WritePPM(out, outImg); 
	printf("%d\n", status); 
	
	fclose(in); 
	fclose(out); 

	FreeImage(&img); 
	FreeImage(&outImg); 

	// Get the time at the end of the execution
	clock_t clock_difference = clock() - start_clock;

	// Convert the time difference into seconds and print
	printf( "Synthesized texture in %.2f(s)\n" , (double)clock_difference/CLOCKS_PER_SEC );
	return 0;
}

