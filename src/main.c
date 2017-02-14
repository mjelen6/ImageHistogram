/*
 * main.c
 *
 *  Created on: 10.02.2017
 *      Author: maciek
 */

#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <FreeImage.h>
#include "my_timers.h"

#define hist_size 256

int main(int argc, char* argv[]){

	int my_rank;							// process rank
	int ntasks;								// number of processes
	int hist_table[hist_size] = { 0 };		// array for histogram

	int tag=0;    /* tag for messages */

	char message[100];        /* storage for message */
	MPI_Status status ;   /* return status for receive */





	/* start up MPI */
	MPI_Init(&argc, &argv);
	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD,&ntasks);




	if (my_rank ==0){



		/* create message */
		printf("Master process - start, id = %d", my_rank);

		FreeImage_Initialise(TRUE);
			FIBITMAP *bitmap = FreeImage_Load(FIF_JPEG, "../img/lenna512x512_gray.jpg",JPEG_DEFAULT);
			if (bitmap){
				unsigned width = FreeImage_GetWidth(bitmap);
				unsigned height = FreeImage_GetHeight(bitmap);
				printf("picture loaded width = %d, height = %d \n", width, height);
				int bytespp = FreeImage_GetLine(bitmap) / width;
				printf(" bytes per pixel = %d \n ", bytespp);
			} else {
				printf("picture not loaded \n ");
			}


	}
	else{
		printf("Hello MPI World From process 0: Num processes: %d\n",p);
		for (source = 1; source < p; source++) {
			MPI_Recv(message, 100, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
			printf("%s\n",message);
		}
	}

	MPI_Reduce( & iLiczba, & iWynik, 1, MPI_INT, MPI_SUM, 0 /*root*/, MPI_COMM_WORLD );
	 printf( "%d, ", iWynik ); //Wszystkie procesy wypisują wynik

	/* shut down MPI */
	MPI_Finalize();
	FreeImage_DeInitialise();


	return 0;
}
