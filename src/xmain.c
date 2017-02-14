/*
 ============================================================================
 Name        : histogram_OMP.c
 Author      : MIS
 Version     :
 Copyright   : Your copyright notice
 Description : Only for image in grayscale
 ============================================================================
 */
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include "my_timers.h"
#include <math.h>
#include "FreeImage.h"

#define NBR_OF_ELEMENTS 40000
#define hist_size 256
#define th_num 4

int mxain() {
	//omp_set_num_threads(th_num);
	int hist_table[hist_size] = { 0 };
	int bytespp;					// bytes per pixel
	FreeImage_Initialise(TRUE);
	int check = 0;

	//load image
	FIBITMAP *bitmap = FreeImage_Load(FIF_JPEG, "lenna512x512_gray.jpg",JPEG_DEFAULT);
	if (bitmap)
		{
		unsigned width = FreeImage_GetWidth(bitmap);
		unsigned height = FreeImage_GetHeight(bitmap);
		printf("picture loaded width = %d, height = %d \n", width, height);

		// Calculate the number of bytes per pixel (3 for 24-bit or 4 for 32-bit)
		bytespp = FreeImage_GetLine(bitmap) / width;
		printf(" bytes per pixel = %d \n ", bytespp);

		// returns a pointer to the data-bits of the bitmap
		BYTE *ptr = FreeImage_GetBits(bitmap);
		if (!ptr){
			printf("no pointer to image \n ");
			return 0;
		}


		start_time();
	//	#pragma omp parallel
		{
			int i,j, hist_priv[hist_size] = {0};
	//		#pragma omp for nowait
			for(i=0;i<height;i++)
			{
				//read line by line from image
				BYTE *bits = (BYTE *)FreeImage_GetScanLine(bitmap, i);
				for(j= 0; j<width;j++)
				{
					hist_priv[bits[j]]++;
				}
			}
			//sum up all hist_priv
		//	#pragma omp critical
			{
				for(i=0; i<hist_size;i++) {
					hist_table[i] += hist_priv[i];
				}
			}
		}
		stop_time();
		for (int i = 0; i < hist_size; i++){
			printf("%d ", hist_table[i]);
			check += hist_table[i];
		}
		printf("\ncheck = %d, nummber of pixel = %d ", check, width*height);
		print_time("\nElapsed:");
		FreeImage_Unload(bitmap);

	} else{
		printf("picture not loaded \n ");
	}
	FreeImage_DeInitialise();
	return 0;
}

