#include <iostream>
#include <io.h>
#include <mpi.h>
#define MCW MPI_COMM_WORLD
using namespace std;

void printTotal(int rank, int data, int result) {
    cout << "Rank: " << rank << " | My Data: " << data << " | total:" << result << endl;
}

void getAllReduce(int rank, int data) {
    int result = 0;
    MPI_Allreduce(&data, &result, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    printTotal(rank, data, result);
    return;
}

void getGather(int rank, int data, int size) {
    int Barray[100];
    int result = 0;

    MPI_Gather(&data, 1, MPI_INT, &Barray, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (!rank) {
        for (int i = 0; i < size; i++) {
            result += Barray[i];
        }
    }
    MPI_Bcast(&result, 1, MPI_INT, 0, MPI_COMM_WORLD);
    printTotal(rank, data, result);
    return;
}

void getSendAndRecieve(int rank, int data, int size) {
    int result = data;
    if (!rank) {
        int rec_size = 1;
        int rec_total;
        while (1) {
            MPI_Recv(&rec_total, 1, MPI_INT, MPI_ANY_SOURCE, 1, MCW, MPI_STATUS_IGNORE);
            result += rec_total;
            rec_size += 1;
            if (rec_size == size) break;
        }

        for (int i = 1; i < size; ++i) {
            MPI_Send(&result, 1, MPI_INT, i, 1, MCW);
        }
        printTotal(rank, data, result);
    }
    if (rank) {
        MPI_Send(&data, 1, MPI_INT, 0, 1, MCW);

        while (1) {
            MPI_Recv(&result, 1, MPI_INT, MPI_ANY_SOURCE, 1, MCW, MPI_STATUS_IGNORE);
            printTotal(rank, data, result);
            break;
        }
    }
    return;
}

int findSquare(int data) {
    int ctr = 0;
    while (data != 2) {
        ctr++;
        data = data / 2;
    }
    return ctr;
}

void getRing(int rank, int data, int size) {
    int result = 0;
    int recvData = data;
    int receive = 0;
    if ((rank - 1) < 0) {
        receive = size-1;
    }
    else {
        receive = (rank - 1);
    }

    for (int i = 0; i < size; ++i) {
        MPI_Send(&recvData, 1, MPI_INT, (rank + 1) % size, 0, MCW);
        MPI_Recv(&recvData, 1, MPI_INT, receive, 0, MCW, MPI_STATUS_IGNORE);
        result += recvData;
    }
    printTotal(rank, data, result);

    return;
}

void getCube(int rank, int data, int size) {
    int result = data;
    int recvData = 0;
    int d = findSquare(size);
    int dest = rank;

    unsigned int mask = 1;

    while (d>=0) {
        dest = dest ^ (mask << d);
        //lower = std::min(rank, dest);

        MPI_Send(&result, 1, MPI_INT, dest, 0, MCW);
        MPI_Recv(&recvData, 1, MPI_INT, dest, 0, MCW, MPI_STATUS_IGNORE);
        //if (rank == 3) { cout << "-- Rank " << rank << " | dest " << dest << " | recvData " << recvData << endl; }
        result += recvData;
       
        d--;
    }

    printTotal(rank, data, result);
    return;
}

int main(int argc, char* argv[])
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    srand(time(NULL) + rank);

    int data = rand() % 10;

    getAllReduce(rank, data);
    getGather(rank, data, size);
    getSendAndRecieve(rank, data, size);

    getRing(rank, data, size);
    getCube(rank, data, size);
    MPI_Finalize();

    return 0;
}