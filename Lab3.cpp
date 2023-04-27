#include <stdio.h>
#include <iostream>
#include "mpi.h"

using namespace std;

const int M = 2;
const int N = 12;

void star(int argc, char* argv[], int* buff_message) {
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
	// cout << size << endl;

	for (int i = 0; i < M; i++) {
		if (rank == 0) {
			cout << "**** M = " << i << " ****" << endl;
			cout << "Buffer message: ";
			for (int i = 0; i < N; i++) {
				buff_message[i] = rand() % 10 - 5;
				cout << "\t" << buff_message[i];
			}
			cout << endl;
		}

		int buff;
		int recvMessage[N];

		for (int i = 0; i < N; i++) {
			recvMessage[i] = NULL;
		}

		MPI_Scatter(buff_message, 1, MPI_INT, &buff, 1, MPI_INT, 0, MPI_COMM_WORLD);
		if (rank == 0)
			cout << "Process 0 send buff_massege for other process!" << endl;
		
		int answer = rank;
		MPI_Gather(&answer, 1, MPI_INT, recvMessage, 1, MPI_INT, 0, MPI_COMM_WORLD);
		cout << "Process " << rank << " get message (" << buff << ") from 0 process" << endl;
		cout << "Process " << rank << " send answer (" << rank << ") to 0 process" << endl;

		if (rank == 0){
			cout << "*** Process 0 get answers from other process!" << endl;
			for (int i = 0; i < N; i++) {
				if (recvMessage[i] != NULL)
					cout << recvMessage[i] << "\t";
			}
		}
			
		
		cout << endl;
	}

	if (rank == 0) {
		time = MPI_Wtime() - start;
		cout << "\n\t\tTIME: " << time << endl;
	}

	MPI_Finalize(); //finish MPI
}


int main(int argc, char* argv[])
{
	srand(time(NULL));

	//int message = rand() % 10;
	int buff_message[N];

	star(argc, argv, buff_message);

	return 0;
}
