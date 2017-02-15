#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mpi.h"
#include <FreeImage.h>
#include "my_timers.h"

#define HIST_SIZE 256

int main(int argc, char* argv[]) {
	int my_rank; /* rank of process */
	int p; /* number of processes */
	int source; /* rank of sender */
	int dest; /* rank of receiver */
	int tag = 0; /* tag for messages */
	char message[100]; /* storage for message */
	MPI_Status status; /* return status for receive */
	int root = 0;
	int bitmap_len, sub_bitmap_len;
	char* sub_bitmap;
	int i; /* uniwersal counter */
	int check = 0; /**/
	int hist_table[HIST_SIZE] = { 0 };
	int sub_hist_table[HIST_SIZE] = { 0 };

	/* start up MPI */

	MPI_Init(&argc, &argv);

	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	//get image
	FreeImage_Initialise(TRUE);
	FIBITMAP *bitmap = FreeImage_Load(FIF_JPEG, "../img/lenna512x512_gray.jpg",
			JPEG_DEFAULT);
	if (!bitmap) {
		printf("nie zaladowano obrazu");
		return 0;
	}

	//image properties
	int width = FreeImage_GetWidth(bitmap);

	int height = FreeImage_GetHeight(bitmap);
	int bytespp = FreeImage_GetLine(bitmap) / width;
	bitmap_len = width * height;
	printf("H%d %d %d", height, bytespp, bitmap_len);

	if (my_rank == root) {

		printf("Hello MPI World From process 0: Num processes: %d\n", p);
		for (source = 1; source < p; source++) {
			MPI_Recv(message, 100, MPI_CHAR, source, tag,
			MPI_COMM_WORLD, &status);
			printf("%s\n", message);
		}

		printf("zaladowano obraz\n");
		printf("picture loaded width = %d, height = %d \n", width, height);
		printf(" bytes per pixel = %d \n ", bytespp);

	} else {

		/* create message */
		sprintf(message, "Hello MPI World from process %d!", my_rank);
		dest = 0;
		/* use strlen+1 so that '\0' get transmitted */
		MPI_Send(message, strlen(message) + 1, MPI_CHAR, dest, tag,
				MPI_COMM_WORLD);

	}

	/* allocate sub_bitmap memory .
	 * Scatter the initial data among the process*/
	sub_bitmap_len = bitmap_len / p;
	sub_bitmap = (char*)malloc(sub_bitmap_len);
	MPI_Scatter(bitmap, sub_bitmap_len, MPI_CHAR, sub_bitmap, sub_bitmap_len,
			MPI_CHAR, root, MPI_COMM_WORLD);

	/*each process calc his own histogram on his amount of bitmaps*/
	for (i = 0; i < sub_bitmap_len; i++) {
		sub_hist_table[sub_bitmap[i]]++;
	}

	/*sum up all sub_histogram's to one */
	MPI_Reduce(sub_hist_table, hist_table, HIST_SIZE, MPI_INT, MPI_SUM, root,
			MPI_COMM_WORLD);

	if (my_rank == root) {
		for (i = 0; i < HIST_SIZE; i++) {
			printf("%d ", hist_table[i]);
			check += hist_table[i];
		}
		printf("\ncheck = %d, nummber of pixel = %d ", check, bitmap_len);
	}

	free(sub_bitmap);

	FreeImage_DeInitialise();
	/* shut down MPI */
	MPI_Finalize();

	return 0;
}
