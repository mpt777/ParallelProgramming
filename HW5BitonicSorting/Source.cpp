#include <iostream>
#include <io.h>
#include <mpi.h>
#define MCW MPI_COMM_WORLD
using namespace std;

int findSquare(int data) {
    int ctr = 0;
    while (data != 2) {
        ctr++;
        data = data / 2;
    }
    return ctr;
}

int max(int data, int recvData) {
    if (data > recvData) {
        return data;
    }
    return recvData;
}

int min(int data, int recvData) {
    if (data < recvData) {
        return data;
    }
    return recvData;
}

bool getFlip(int rank, int offset) {
    for (int i = rank; i >= 0; --i) {
        if (i % offset == 0 and i) {
            return true;
        }
    }
    return false;
}

void bitonicSort(int rank, int data, int size) {
    int max_d = findSquare(size);
    int d = 0;
    int dest = rank;
    int recvData;
    bool flip = false;
    //int array[8];
    //int recvArray[8];

    unsigned int mask = 1;

    //cout << "Init | Rank: " << rank << " | Data: " << data << endl;
    
    while (d <= max_d) {
       
        dest = rank ^ (mask << d);
        //cout << "power "<< pow(2, (d+1)) << endl;
        flip = getFlip(rank, pow(2, (d+1)));
   
        cout << "Rank " << rank << " | Dest " << dest << " | Data: " << data << " | flip " << flip << endl;
        MPI_Send(&data, 1, MPI_INT, dest, 0, MCW);
        MPI_Recv(&recvData, 1, MPI_INT, dest, 0, MCW, MPI_STATUS_IGNORE);
        if (flip) {
            if (rank > dest) {
                data = min(data, recvData);
            }
            else {
                data = max(data, recvData);
            }
        }
        else {
            if (rank < dest) {
                data = min(data, recvData);
            }
            else {
                data = max(data, recvData);
            }
        }
        d++;
    }
    d--;
    while (d >= 0) {

        dest = rank ^ (mask << d);
        //cout << "power "<< pow(2, (d+1)) << endl;
        cout << "Rank " << rank << " | Dest " << dest << " | Data: " << data <<  endl;
        MPI_Send(&data, 1, MPI_INT, dest, 0, MCW);
        MPI_Recv(&recvData, 1, MPI_INT, dest, 0, MCW, MPI_STATUS_IGNORE);
        if (rank < dest) {
            data = min(data, recvData);
        }
        else {
            data = max(data, recvData);
        }
        d--;
    }
    cout << "Final | Rank: " << rank << " | Data: " << data << endl;

    return;
}

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    srand(time(NULL) - rank + rand()%100);

    int data = rand() % 100;

    bitonicSort(rank, data, size);
    MPI_Finalize();

    return 0;
}