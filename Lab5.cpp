#include <stdio.h>
#include <time.h>
#include <random>
#include <iostream>
#include "mpi.h"

#define N 1

using namespace std;

random_device rd;
mt19937 gen(rd());

/// <summary>
/// функция, генерирующая рандомное число в заданном промежутке
/// </summary>
/// <param name="low"></param>
/// <param name="high"></param>
/// <returns></returns>
int random(int low, int high)
{
	std::uniform_int_distribution<> dist(low, high);
	return dist(gen);
}

/// <summary>
/// выво массива в консоль
/// </summary>
/// <param name="mas"></param>
/// <param name="len"></param>
void print(int* mas, int len) {
	cout << "\t\t";
	for (int i = 0; i < len; i++) {
		cout << mas[i] << "\t";
	}
	cout << endl;
}

int main(int* argc, char** argv)
{
	MPI_Status status;

	int size; //кол-во рангов
	int rank; //ранг
	int dims[N]; //dims - массив длины ndims, задает количество процессов в каждом измерении решетки
	int periods[N];	// periods - массив длины ndims, определяет, является ли решетка периодической вдоль каждого измерения

	MPI_Init(argc, &argv); //инициализация
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	//int mid = random(1, size - 1); // рандомно вычисляем ранг, относительно которого разделим множество процессов
	int mid = size / 2; // рандомно вычисляем ранг, относительно которого разделим множество процессов
	int size_left = mid; // размер левой части (до mid)
	int size_right = size - size_left; // размер правой части, начиная с mid ранга
	int* left_group_rank = new int[size_left]; // ранги для 1 группы
	int* right_group_rank = new int[size_right]; // ранги для 2 группы

	for (int i = 0; i < size; i++) {
		left_group_rank[i] = i;
		right_group_rank[i] = i + mid;
	}

	MPI_Group mainGroup; // базовая группа
	MPI_Comm Main_Comm; //
	MPI_Comm DECART_COMM; // 1ая группа с декартовой топологией
	MPI_Comm GRAPH_COMM; // 2ая группа с топологией графа
	MPI_Comm Dec_Comm;
	MPI_Comm Graph_Comm;
	MPI_Group groupLeft;
	MPI_Group groupRight;

	MPI_Comm_dup(MPI_COMM_WORLD, &Main_Comm); // дублирует существующий коммуникатор со всей его кешированной информацией
	MPI_Comm_group(Main_Comm, &mainGroup); // доступ к группе, связанной с данным коммуникатором
	
	dims[0] = 0; // начальные параметры для решетки
	periods[0] = 1;

	MPI_Group_incl(mainGroup, size_left, left_group_rank, &groupLeft); // создаем "левую" группу процессов и коммуникатор для нее
	MPI_Comm_create(Main_Comm, groupLeft, &Dec_Comm);
	MPI_Group_incl(mainGroup, size_right, right_group_rank, &groupRight); // создаем "правую" группу процессов и коммуникатор для нее
	MPI_Comm_create(Main_Comm, groupRight, &Graph_Comm);

	int index[] = { 1, 2, 3, 4, 9, 10 }; //количество входящих ребер в вершину ( 0 - 1; 1 - 1; 2 - 1 ... )
	int edges[] = { 4, 4, 4, 4, 0, 1, 2, 3, 5, 4 }; // концы ребер (0 <-> 4 ... )
	int source; // источник отправки
	int rank_dest; // ранг процесса назначения (целое число)
	int senderInfo; // сообщение

	//вывод основной информации
	if (rank == 0) {
		cout << "~~~~~~~~~~" << endl;
		cout << "Mid: " << mid << endl;
		cout << "Size: " << size << endl;
		cout << "Left group: " << endl;
		print(left_group_rank, size_left);
		cout << "Left group: " << endl;
		print(right_group_rank, size_right);
		cout << "~~~~~~~~~~" << endl;
	}

	if (rank < mid) // если левая часть
	{
		MPI_Dims_create(size_left, N, dims); // распределяем все процессы в n-мерную топологическую среду
		MPI_Cart_create(Dec_Comm, N, dims, periods, 0, &DECART_COMM); // создаем решетку
		MPI_Cart_shift(DECART_COMM, 0, 1, &source, &rank_dest); // Циклический сдвиг на k элементов вдоль измерения решетки 

		MPI_Sendrecv(&rank, 1, MPI_INT, rank_dest, rank_dest, &senderInfo, 1, MPI_INT, source, rank, DECART_COMM, &status); // рассылает и получает сообщение

		cout << "*DEC*\n\t" << rank << " <--- " << senderInfo << endl;

	}
	else { // если правая часть
		rank -= mid;
		int count_neighbors; // количестово соседей
		MPI_Graph_create(Graph_Comm, 6, index, edges, 1, &GRAPH_COMM); // создание коммуникатора с топологией типа граф
		MPI_Graph_neighbors_count(GRAPH_COMM, rank, &count_neighbors); // возвращает количество соседей узла, связанного с топологией графа

		int* ranks = new int[count_neighbors];
		MPI_Graph_neighbors(GRAPH_COMM, rank, count_neighbors, ranks); // соседние ранги

		for (int i = 0; i < count_neighbors; i++) {
			MPI_Sendrecv(&ranks[i], 1, MPI_INT, ranks[i], 0, &senderInfo, 1, MPI_INT, ranks[i], 0, GRAPH_COMM, &status); // рассылает и получает сообщение
			cout << "@GRAPH@\n\t" << ranks[i] << " <---> " << rank << endl;
		}
	}
	MPI_Finalize();
}
