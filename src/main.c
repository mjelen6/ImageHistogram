#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mpi.h"
#include <FreeImage.h>
#include "my_timers.h"

#define HIST_SIZE 256
#define ROOT 0


int main(int argc, char* argv[]) {
	int my_rank; /* rank of process */
	int p; /* number of processes */

	int bitmap_len, sub_bitmap_len;

	unsigned char* bitmap;
	unsigned char* sub_bitmap;

	int i; /* uniwersal counter */
	int check = 0; /**/
	int hist_table[HIST_SIZE] = {0};
	int sub_hist_table[HIST_SIZE] = {0};

	/* start up MPI */
	MPI_Init(&argc, &argv);

	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &p);



	if (my_rank == ROOT) {

		//get image
		FreeImage_Initialise(TRUE);
		FIBITMAP * fibitmap = FreeImage_Load(FIF_JPEG, "../img/lenna512x512_gray.jpg", JPEG_DEFAULT);
		if (!fibitmap) {
			printf("Error during image loading");
			FreeImage_DeInitialise();
			MPI_Finalize();
			return -1;
		}

		//image properties
		int width = FreeImage_GetWidth(fibitmap);
		int height = FreeImage_GetHeight(fibitmap);
		int bytespp = FreeImage_GetLine(fibitmap) / width;
		bitmap_len = width * height;
		bitmap = FreeImage_GetBits(fibitmap);



		sub_bitmap_len = bitmap_len / p;

		printf("picture loaded, width = %d, height = %d \n", width, height);
		printf("bytes per pixel = %d \n ", bytespp);

	}


	MPI_Bcast(&sub_bitmap_len, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

	/* Allocate sub_bitmap memory .
	 * Scatter the initial data among the process*/
	sub_bitmap = (unsigned char*)malloc(sub_bitmap_len);
	MPI_Scatter(bitmap, sub_bitmap_len, MPI_CHAR, sub_bitmap, sub_bitmap_len, MPI_CHAR, ROOT, MPI_COMM_WORLD);

	/*each process calc his own histogram on his amount of bitmaps*/
	for (i = 0; i < sub_bitmap_len; i++) {
		sub_hist_table[sub_bitmap[i]]++;
	}

	/*sum up all sub_histogram's to one */
	MPI_Reduce(sub_hist_table, hist_table, HIST_SIZE, MPI_INT, MPI_SUM, ROOT, MPI_COMM_WORLD);

	if (my_rank == ROOT) {
		for (i = 0; i < HIST_SIZE; i++) {
			check += hist_table[i];
		}
		printf("\ncheck = %d, nummber of pixel = %d ", check, bitmap_len);

		/* Save histogram to file */
		FILE* f = fopen("data.txt", "w");
		for (i = 0; i < HIST_SIZE; i++) {
			fprintf(f,"%d\n",hist_table[i]);
		}
		fclose(f);
		/* Plot results */
		system("gnuplot -p -e \"plot 'data.txt'\"");

		FreeImage_DeInitialise();
	}

	free(sub_bitmap);

	/* shut down MPI */
	MPI_Finalize();

	return 0;
}
