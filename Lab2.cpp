#include <stdio.h>
#include <iostream>
#include "mpi.h"

using namespace std;

const int M = 1;

void star(int argc, char* argv[], int message) {
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
		cout << "\n\ncall - " << i + 1 << endl;
		if (rank == 0) {
			MPI_Bcast(&message, 1, MPI_INT, 0, MPI_COMM_WORLD); // send message to all process (rank != 0)
			cout << "\n**** Send '" << message << "'message to all process ****" << endl;
			int answer;
			MPI_Reduce(&rank, &answer, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD); // get message from all process
			cout << "Process 0 get answer '" << answer << "' from all process" << endl;
		}
		else {
			int recvMessage;

			MPI_Bcast(&recvMessage, 1, MPI_INT, 0, MPI_COMM_WORLD);
			cout << "Process " << rank << " get message '" << recvMessage << "' from 0 process" << endl;

			int ansMessage = recvMessage + rank;
			MPI_Reduce(&ansMessage, &recvMessage, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
			cout << "Process " << rank << " send message '" << ansMessage << "' to 0 process" << endl;
		}
	}

	time = MPI_Wtime() - start;

	cout << "\nTIME: " << time << endl;

	MPI_Finalize(); //finish MPI
}


int main(int argc, char* argv[])
{
	srand(time(NULL));

	int message = rand() % 10;
	star(argc, argv, message);

	return 0;
}
