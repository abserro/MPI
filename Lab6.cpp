#include <stdio.h>
#include <time.h>
#include <random>
#include <iostream>
#include "mpi.h"

const int N = 1;

using namespace std;

// на 12 потоков

int main(int* argc, char** argv)
{
	int rank; //ранг
	int size; //кол-во рангов
	int dims[N]; //dims - массив длины ndims, задает количество процессов в каждом измерении решетки
	int periods[N];	// periods - массив длины ndims, определяет, является ли решетка периодической вдоль каждого измерения

	MPI_Status status;

	MPI_Init(argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int sizeGroup = size / 2; // размер каждой из групп

	MPI_Group group;
	MPI_Comm comm;

	//декарт. топология
	MPI_Comm DEC_COM; // 1ая группа с декартовой топологией

	// задаём данные для решётки
	for (int i = 0; i < N; i++) {
		dims[i] = 0;
		periods[i] = 1;
	}

	// граф
	MPI_Comm GRAPH_COM; // 2ая группа с топологией графа
	MPI_Comm graph_com_first;
	MPI_Comm graph_com_second;

	MPI_Group group_first;
	MPI_Group group_second;

	MPI_Comm_dup(MPI_COMM_WORLD, &comm);
	MPI_Comm_group(comm, &group);

	int* rank_group_first = new int[sizeGroup]; // ранги для 1 группы
	int* rank_group_second = new int[sizeGroup]; // ранги для 2 группы
	for (int i = 0; i < sizeGroup; i++) {
		rank_group_first[i] = i;
		rank_group_second[i] = sizeGroup + i;
	}

	MPI_Group_incl(group, sizeGroup, rank_group_first, &group_first);
	MPI_Comm_create(comm, group_first, &graph_com_first);

	MPI_Group_incl(group, sizeGroup, rank_group_second, &group_second);
	MPI_Comm_create(comm, group_second, &graph_com_second);

	int index[] = { 5, 6, 7, 8, 9, 10 }; // количество соседей у каждой вершины
	int edges[] = { 1, 2, 3, 4, 5, 0, 0, 0, 0, 0 }; // последовательный список дуг графа

	int source; // источник
	int dst; // ранг процесса назначения
	int reorder = 0; //
	int data; // полученное сообщение

	if (rank < sizeGroup)
	{
		MPI_Dims_create(sizeGroup, N, dims); // создаем n-мерную топологическую среду
		MPI_Cart_create(graph_com_first, N, dims, periods, reorder, &DEC_COM);
		MPI_Cart_shift(DEC_COM, 0, 1, &source, &dst); // циклический сдвиг

		MPI_Sendrecv(&rank, 1, MPI_INT, dst, dst, &data, 1, MPI_INT, source, rank, DEC_COM, &status);

		cout << "Decart topology: \n\t" << "Process " << rank << " get data < " << data << " > from " << data << " process" << endl;
	}
	else {
		MPI_Graph_create(graph_com_second, 6, index, edges, 1, &GRAPH_COM); // создаем коммуникатор топологии графа

		rank -= sizeGroup;
		int count_neighbors; // количестово соседей
		MPI_Graph_neighbors_count(GRAPH_COM, rank, &count_neighbors);

		int* ranks = new int[count_neighbors];
		MPI_Graph_neighbors(GRAPH_COM, rank, count_neighbors, ranks);

		for (int i = 0; i < count_neighbors; i++) {
			MPI_Sendrecv(&ranks[i], 1, MPI_INT, ranks[i], 0, &data, 1, MPI_INT, ranks[i], 0, GRAPH_COM, &status);
			cout << "Graph topology: \n\t" << "Process " << ranks[i] << " get data < " << data << " > from " << rank << " process" << endl;
		}
	}
	MPI_Finalize();
}

