#include <stdio.h>
#include <iostream>
#include "mpi.h"

using namespace std;

const int M = 5;

void star(int argc, char* argv[], int sendMessage) {
	int size;
	int rank;
	double finish;
	double start;
	double time;
	MPI_Status status;
	MPI_Init(&argc, &argv); //start MPI

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	start = MPI_Wtime();

	for (int i = 0; i < M; i++) {
		if (rank == 0) {
			cout << "\n\n*** (" << i << ") get message from star! ***" << endl;
			for (int numProcess = 1; numProcess < size; numProcess++) {
				cout << "Send message '" << sendMessage << "' from 0 to " << numProcess << " process..." << endl;
				MPI_Send(&sendMessage, 1, MPI_INT, numProcess, numProcess, MPI_COMM_WORLD); // send to all process
				int answer;
				MPI_Recv(&answer, 1, MPI_INT, numProcess, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				cout << "\tGet answer '" << answer << "' from " << status.MPI_SOURCE << " process" << endl;
			}
		}
		else {
			int recvMessage;
			MPI_Recv(&recvMessage, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status); // get message from 0 process
			int ansMessage = recvMessage + rank;
			MPI_Send(&ansMessage, 1, MPI_INT, 0, 0, MPI_COMM_WORLD); // send answer to 0 process

		}
	}

	time = MPI_Wtime() - start;

	MPI_Finalize(); //finish MPI

	cout << "TIME: " << time << endl;
}


int main(int argc, char* argv[])
{
	srand(time(NULL));

	int message = rand() % 10;
	star(argc, argv, message);

	return 0;
}
