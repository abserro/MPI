#include <stdio.h>
#include "mpi.h"
#include <iostream>
#include <random>

using namespace std;

const int ksizeMassive = 422; // количество полиномов
const int klenFirst = 10; // длина первого полинома
const int klenSecond = 11; // длина второго полинома


struct Polynomials {
    int pol_first[klenFirst]; 
    int pol_second[klenSecond];
};

/// <summary>
/// Функция, осуществляющая умножение двух полиномов
/// </summary>
/// <param name="first"></param>
/// <param name="second"></param>
/// <returns></returns>
int* multPolynomials(int* polFirst, int* polSecond) {
    int* result = new int[klenFirst + klenSecond];

    for (int i = 0; i < klenFirst + klenSecond; i++)
        result[i] = 0;

    for (int i = klenFirst - 1; i != -1; i--)
        for (int j = klenSecond - 1; j != -1; j--)
            result[i + j + 1] += polFirst[i] * polSecond[j];
      
    return result;
}

int main(int argc, char* argv[])
{
    int ProcNum;
    int ProccesRank;

    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProccesRank);

    Polynomials* polynomials = new Polynomials[ksizeMassive]; // все полиномы
    Polynomials* dataRecv = new Polynomials[(int)(ksizeMassive / ProcNum)]; // результат умножения

    // инициализация типа
    MPI_Datatype MPI_POLY; 
    MPI_Datatype type[2] = { MPI_INT, MPI_INT };
    MPI_Aint adress[] = { offsetof(Polynomials, pol_first), offsetof(Polynomials, pol_second) };

    int lenMass[2] = { klenFirst, klenSecond };

    MPI_Type_create_struct(2, lenMass, adress, type, &MPI_POLY);
    MPI_Type_commit(&MPI_POLY);

    //заполнение полиномов:
    if (ProccesRank == 0)
        for (int i = 0; i < ksizeMassive; i++) {
            for (int j = 0; j < klenFirst; j++)
                polynomials[i].pol_first[j] = rand() % 10 ;
            for (int j = 0; j < klenSecond; j++)
                polynomials[i].pol_second[j] = rand() % 10;

        }

    MPI_Scatter(polynomials, (int)(ksizeMassive / ProcNum), MPI_POLY, dataRecv, (int)(ksizeMassive / ProcNum), MPI_POLY, 0, MPI_COMM_WORLD); // 0 процесс раздает всем остальным процессам часть вычислений

    cout << "Process #" << ProccesRank << ": " << endl;
    for (int i = 0; i < (int)(ksizeMassive / ProcNum); i++) {
        int* mas = multPolynomials(dataRecv[i].pol_first, dataRecv[i].pol_second);
        for (int j = 0; j < klenFirst + klenSecond; j++)
            cout << mas[j] << " ";
        cout << "\n" << " ";
    }
    MPI_Finalize();
}

