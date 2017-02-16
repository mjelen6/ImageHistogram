/*
 ============================================================================
 Name        : MPI_Histogram_MM.c
 Author      : Michal
 Version     :
 Copyright   : Your copyright notice
 Description : Hello MPI World in C
 ============================================================================
 */
#include <stdio.h>
#include <string.h>
#include "mpi.h"
#include <FreeImage.h>
#include "my_timers.h"

#define hist_size 256

int main(int argc, char* argv[]) {
	int my_rank; /* rank of process */
	int p; /* number of processes */
	int root = 0;
	int bitmap_len, sub_bitmap_len;
	unsigned char *sub_bitmap;
	int i; /* uniwersal counter */
	int check = 0; /**/
	int hist_table[hist_size] = { 0 };
	int sub_hist_table[hist_size] = { 0 };

	/* start up MPI */
	MPI_Init(&argc, &argv);

	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	FreeImage_Initialise(TRUE);
	FIBITMAP *bitmap = NULL; /* pointer to image*/
	BYTE *ptr = NULL;

	if (my_rank == root) {
		//get image
		bitmap = FreeImage_Load(FIF_JPEG, "../img/lenna512x512_gray.jpg",
				JPEG_DEFAULT);
		if (!bitmap) {
			printf("nie zaladowano obrazu");
			return 0;
		}

		// returns a pointer to the data-bits of the bitmap
		ptr = FreeImage_GetBits(bitmap);
		if (!ptr) {
			printf("no pointer to image \n ");
			return 0;
		}

		//image properties
		unsigned width = FreeImage_GetWidth(bitmap);
		unsigned height = FreeImage_GetHeight(bitmap);
		int bytespp = FreeImage_GetLine(bitmap) / width;
		bitmap_len = width * height;
		sub_bitmap_len = bitmap_len / p;

		printf("Hello MPI World From process 0: Num processes: %d\n", p);
		printf("picture loaded width = %d, height = %d \n", width, height);
		printf(" bytes per pixel = %d \n ", bytespp);

	}

	MPI_Bcast(&sub_bitmap_len, 1, MPI_INT, root, MPI_COMM_WORLD);

	/* allocate sub_bitmap memory .
	 * Scatter the initial data among the process*/
	sub_bitmap = (char *) malloc(sub_bitmap_len);
	MPI_Scatter(ptr, sub_bitmap_len, MPI_CHAR, sub_bitmap, sub_bitmap_len,
			MPI_CHAR, root, MPI_COMM_WORLD);

	/*each process calc his own histogram on his amount of bitmaps*/
	for (i = 0; i < sub_bitmap_len; i++) {
		sub_hist_table[sub_bitmap[i]]++;
	}

	//free alocated mem
	free(sub_bitmap);

	/*sum up all sub_histogram's to one */
	MPI_Reduce(sub_hist_table, hist_table, hist_size, MPI_INT, MPI_SUM, root,
			MPI_COMM_WORLD);

	if (my_rank == root) {
		for (i = 0; i < hist_size; i++) {
			printf("%d ", hist_table[i]);
			check += hist_table[i];
		}
		printf("\ncheck = %d, nummber of pixel = %d ", check, bitmap_len);

		FILE* f = fopen("data.txt", "w");
		for (i = 0; i < hist_size; i++) {
			fprintf(f, "%d\n", hist_table[i]);
		}
		fclose(f);
		system("gnuplot -p -e \"plot 'data.txt'\"");
	}

	FreeImage_DeInitialise();
	/* shut down MPI */
	MPI_Finalize();

	return 0;
}
