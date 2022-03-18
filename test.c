#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "image.h"
#include "ppm.h"
#include "texture_synthesis.h"

int main() {
	FILE *in = fopen("data/text3.ppm", "rb"); 
	FILE *out = fopen("results/text3out.ppm", "wb"); 

	Image *img = ReadPPM(in); 
	// getNeighborhoodWindow(img, 20, 20, 3, 10, 10); 
	Image *outImg = SynthesizeFromExemplar(img, 128, 128, 2, 0); 

	WritePPM(out, outImg); 

	free(img); 
	free(outImg); 
	
	fclose(in); 
	fclose(out); 
    return 0;
}