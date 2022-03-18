#ifndef TEXTURE_SYNTHESIS_H
#define TEXTURE_SYNTHESIS_H
#include "image.h"

/** A struct storing information about a to-be-synthesized pixel*/
typedef struct
{
	/** The index of the pixel to be synthesized*/
	PixelIndex idx;
	
	/** The number of neighboring pixels in a set state*/
	unsigned int neighborCount;
	
	/** A member for storing a value used to resolve ties*/
	unsigned int r;
} TBSPixel;

/** A function that compares two TBSPixels and returns a negative number if the first should come earlier in the sort order and a positive number if it should come later*/
int CompareTBSPixels( const void *v1 , const void *v2 );

/** A function that sorts an array of TBSPixels*/
int SortTBSPixels( TBSPixel *tbsPixels , unsigned int sz );

int countNeighbors(Image *synthesized, int x, int y); 

int getTBSPixels(TBSPixel *TBSPixels, Image *synthesized); 

int getNeighborhoodWindow(Pixel *window, Image *synthesized, int windowRadius, int x, int y); 

double compareWindows(Pixel *TBSWindows, Pixel *xWindow, int windowRadius); 

int findMatches(Pixel *pixelMatches, Pixel *TBSWindow, Image *synthesized, int windowRadius); 

Pixel RandomPick(Pixel *pixels, int numPixels);

/** A function that extends the exemplar into an image with the specified dimensions, using the prescribed window radius -- the verbose argument is passed in to enable logging to the command prompt, if desired*/
Image *SynthesizeFromExemplar( const Image *exemplar , unsigned int outWidth , unsigned int outHeight , unsigned int windowRadius , bool verbose );

#endif // TEXTURE_SYNTHESIS_H