#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include "texture_synthesis.h"
#include "image.h"

// compares tbs pixels 
int CompareTBSPixels(const void *v1, const void *v2) {
	const TBSPixel *tp1 = (const TBSPixel *)v1;
	const TBSPixel *tp2 = (const TBSPixel *)v2;
	int d = tp1->neighborCount - tp2->neighborCount;
	if( d ) return -d;
	d = tp1->idx.y - tp2->idx.y;
	if( d ) return d;
	d = tp1->idx.x - tp2->idx.x;
	if( d ) return d;
	return tp2->r - tp1->r;
}

// sorts tbs pixels, returns zero if succeeded
int SortTBSPixels( TBSPixel *tbsPixels , unsigned int sz )
{
	unsigned int *permutation = (unsigned int*)malloc( sizeof(unsigned int)*sz );
	if( !permutation )
	{
		fprintf( stderr , "[ERROR] Failed to allocate memory for permutation: %d\n" , sz );
		return 1;
		exit(1);
	}
	for( unsigned int i=0 ; i<sz ; i++ ) permutation[i] = i;
	for( unsigned int i=0 ; i<sz ; i++ )
	{
		unsigned int i1 = rand() % sz;
		unsigned int i2 = rand() % sz;
		unsigned int tmp = permutation[i1];
		permutation[i1] = permutation[i2];
		permutation[i2] = tmp;
	}

	for( unsigned int i=0 ; i<sz ; i++ ) tbsPixels[i].r = permutation[i];
	free( permutation );

	qsort( tbsPixels , sz , sizeof( TBSPixel ) , CompareTBSPixels );

	return 0;
}

// returns number of set pixels in a 3 * 3 window around pixel of interest
int countNeighbors(Image *synthesized, int col, int row) {

	int count = 0; 
	for (int i = col-1; i < col+2; ++i) { // look from x-1 to x+1
		for (int j = row-1; j < row+2; ++j) { // look from y-1 to y+1
			if (i == col && j == row) continue;
			PixelIndex pxlIdx = {i,j}; 
			if (InBounds(synthesized, pxlIdx) && col >= 0 && row >= 0) {
				int isSet = GetPixel(synthesized, pxlIdx)->a; 
				if (isSet) count++; 
			}
		}
	}
	return count; 
}

// put all TBS pixels in an array and return number of TBS pixels
int getTBSPixels(TBSPixel *TBSPixels, Image *synthesized) {
	int TBSCount = 0; 
	int neighborCount;
	unsigned int width = synthesized->width;
	unsigned int height = synthesized->height;

	
	// look for all pixels in synthesized image for TBS without considering exemplar pixels
	for (unsigned int row = 0; row < height; ++row) { // iterate entire row of Image synthesized
		for (unsigned int col = 0; col < width; ++col) { // same with columns
			if (synthesized->pixels[row*width+col].a == 0) { //
				neighborCount = countNeighbors(synthesized, col, row); 
				if (neighborCount == 0) continue; // not TBS
				TBSPixels[TBSCount].idx.x = col; // indexed by count? 
				TBSPixels[TBSCount].idx.y = row;
				TBSPixels[TBSCount].neighborCount = neighborCount; 
				++TBSCount;
			}
		}
	}
	return TBSCount; 
}

int getNeighborhoodWindow(Pixel *window, Image *img, int windowRadius, int x, int y) {
	int count = 0; // 0 based indexing of window Pixels 
	Pixel defaultPixel = {0,0,0,0}; 
	Pixel *currPxl;
	for (int col = x-windowRadius; col < x+windowRadius+1; ++col) {
		for (int row = y-windowRadius; row < y+windowRadius+1; ++row) {
			PixelIndex pxlIdx = {col, row}; 
			if (col >= 0 && row >= 0 && InBounds(img, pxlIdx)) {
				currPxl = GetPixel(img, pxlIdx);
				if (currPxl->a) {  
					window[count++] = *currPxl;
				} else window[count++] = defaultPixel; // !isSet
			} else window[count++] = defaultPixel; // !isSet
		}
	}
	return 0; 
}

double compareWindows(Pixel *TBSWindow, Pixel *xWindow, int windowRadius) {
	int windowLength = windowRadius*2+1;
	double Sigma = windowLength / 6.4;
	double diff = 0.0;

	for (int row = -windowRadius; row <= windowRadius; ++row) {
		for (int col = -windowRadius; col <= windowRadius; ++col) {
			if (row == 0 && col == 0) continue; // skip TBSPixel 
			Pixel TBSPxl = TBSWindow[(windowRadius+row)*windowLength + windowRadius + col]; 
			Pixel exemPxl = xWindow[(windowRadius+row)*windowLength + windowRadius + col];
			if (TBSPxl.a) { 
				if (exemPxl.a == 0) return -1; // not a candidate  
				double d = PixelSquaredDifference(TBSPxl, exemPxl); 
				double s = exp(-(pow(col, 2)+pow(row, 2))/(2*pow(Sigma, 2)));
				diff += d*s;
			} 
		}
	}
	return diff; 
}

int findMatches(Pixel *pixelMatches, Pixel *TBSWindow, Image *exemplar, int windowRadius) {
	const unsigned int exemplarWidth = exemplar->width; 
	const unsigned int exemplarHeight = (exemplar->height); 
	unsigned int windowLength = windowRadius * 2 + 1; 

	float minDiff = pow(2,52); 
	float diff = -3; 
	float *diffArray = malloc(sizeof(float)*exemplarWidth*exemplarHeight); // fake 2d array 
	for (unsigned int row = 0; row < exemplarHeight; ++row) { // iterate through entire image 
		for (unsigned int col = 0; col < exemplarWidth; ++col) {
			Pixel *xWindow = malloc(sizeof(Pixel)*windowLength*windowLength); 
			getNeighborhoodWindow(xWindow, exemplar, windowRadius, col, row); 
			diff = compareWindows(TBSWindow, xWindow, windowRadius);
			free(xWindow); 
			diffArray[row*exemplarWidth+col] = diff; 
			if (diff < 0) continue; // skip comparison since not candidate
			if (diff < minDiff) minDiff = diff;  
		}
	}

	// defining threshold 
	minDiff *= 1.1; 
	// go through exemplar again
	int count = 0; 
	for (unsigned int row = 0; row < exemplarHeight; ++row) { // iterate through entire image 
		for (unsigned int col = 0; col < exemplarWidth; ++col) {
			float diff = diffArray[row*exemplarWidth+col]; 
			if (diff < 0) continue; // skip since not candidate 
			PixelIndex pxlIdx = {col, row};
			if (diff <= minDiff && InBounds(exemplar, pxlIdx)) { // might equal if minDiff=0 for perfect match 
				pixelMatches[count++] = *GetPixel(exemplar, pxlIdx);
			} 
		}
	}
	free(diffArray); 
	return count;
}

// returns a random pixel from an array of pixels
Pixel RandomPick(Pixel *pixels, int numPixels){
	return pixels[rand() % (numPixels)];
}

Image *SynthesizeFromExemplar( const Image *exemplar , unsigned int outWidth , unsigned int outHeight , unsigned int windowRadius , bool verbose ) {
	printf("entered synthesizefromexemplar\n");
	//TODO: figure out what verbose does
	assert(verbose == false);

	unsigned int exemplarWidth = exemplar->width; 
	unsigned int exemplarHeight = exemplar->height; 
	
	// return exemplar if dimensions are the same
	if(exemplarWidth == outWidth && exemplarHeight == outHeight) return (Image *) exemplar;

	// ensure dimensions of synthesized are sufficient for exemplar
	assert(exemplarWidth <= outWidth && exemplarHeight <= outHeight);

	// create an empty synthesized image
	Image *synthesized = NULL;
	synthesized = AllocateImage(outWidth, outHeight);
	printf("allocated an image for synthesized\n");
	
	// add exemplar to synthesized image if dimensions allow
	PixelIndex pxlIdx;
	for(pxlIdx.x = 0; pxlIdx.x < exemplarWidth; ++(pxlIdx.x)){
		for(pxlIdx.y = 0; pxlIdx.y < exemplarHeight; ++(pxlIdx.y)){
			*GetPixel(synthesized, pxlIdx) = *GetPixel((Image *) exemplar, pxlIdx);
		}
	}
	printf("put exemplar in synthesized\n");

	// create pointer to array of TBS pixels
	TBSPixel *tbsPixels = malloc(sizeof(TBSPixel) * outWidth*outHeight); 
	if (!tbsPixels) return NULL; 
	printf("created pointer to TBS pixels\n");

	// find and store TBS pixels 
	int tbsCount = getTBSPixels(tbsPixels, synthesized); 
	if (!tbsPixels) printf("Error: getTBSPixels failed\n"); 
	printf("found number of TBS\n");

	// sort TBS pixels 
	int isSorted = SortTBSPixels(tbsPixels, tbsCount); 
	if (isSorted != 0) printf("Error: SortTBSPixels failed\n"); 
	printf("sorted TBS pixels\n");

	// declare variables to locate current pixel
	int col;
	int row;
	//int matches;
	
	printf("tbsCount is: %d\n", tbsCount); 
	while (tbsCount) { // image not filled 
		for (int i = 0; i < tbsCount; ++i) {
			TBSPixel currTBSPixel = tbsPixels[i];
			col = (int) currTBSPixel.idx.x;
			row = (int) currTBSPixel.idx.y;
			// get neighbor window of tbs pixel
			Pixel *TBSWindow = malloc(sizeof(Pixel)*(2*windowRadius+1)*(2*windowRadius+1));
			getNeighborhoodWindow(TBSWindow, synthesized, windowRadius, col, row);
			// construct pixel array for matches
			Pixel *pixelMatches = malloc(sizeof(Pixel)*exemplarWidth*exemplarHeight);
			int matches = findMatches(pixelMatches, TBSWindow, (Image *) exemplar, windowRadius); 
			//assert(matches != 0); 
			Pixel bestMatch = RandomPick(pixelMatches, matches);
			PixelIndex pxlIdx = {col, row};
			*GetPixel(synthesized, pxlIdx) = bestMatch;
			free(TBSWindow);
			free(pixelMatches);
		}
		printf("got out of for loop!\n");
		// get new TBSPixels array 
		tbsCount = getTBSPixels(tbsPixels, synthesized);
		printf("tbsCount is: %d\n", tbsCount); 
		// sort TBS pixels
		int isSorted = SortTBSPixels(tbsPixels, tbsCount);
		if (isSorted != 0) printf("Error: SortTBSPixels failed");
	}
	free(tbsPixels); 
	return synthesized;
}