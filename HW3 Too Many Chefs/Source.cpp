#include <iostream>
#include <Windows.h>
#include <mpi.h>
#define MCW MPI_COMM_WORLD
using namespace std;
int main(int argc, char** argv) {
    int rank, size;
    int data;
    int order_queue = 0;
    MPI_Request request;
    MPI_Status status;
    int flag = 0;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);
    //    MPI_Send(&rank,1,MPI_INT,(rank+1)%size,0,MCW);
    //    MPI_Recv(&data,1,MPI_INT,MPI_ANY_SOURCE,0,MCW,MPI_STATUS_IGNORE);
    if (!rank) {
        // I am cook
        while (order_queue < 20) {
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MCW, &flag, &status);
            while (flag) {
                MPI_Recv(&data, 1, MPI_INT, MPI_ANY_SOURCE, 1, MCW, MPI_STATUS_IGNORE);
                MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MCW, &flag, &status);
                order_queue++;
            }
            cout << "Order Queue " << order_queue << endl;
            if (order_queue) {
                 // cooking!
                order_queue--;
                cout << "Cook: I gotta cook! " << endl;
                Sleep(1000);
            }
            else {
                cout << rank << ": " << "420." << endl;
                Sleep(1000);
            }
        }
        // recieve all outstanding orders
        while (flag) {
            MPI_Recv(&data, 1, MPI_INT, MPI_ANY_SOURCE, 1, MCW, MPI_STATUS_IGNORE);
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MCW, &flag, &status);
        }
        for (int i = 1; i < size; i++) {
            cout << i << " You Friking Frick! When will you learn that you actions have consequences!" << endl;
            MPI_Send(&rank, 1, MPI_INT, i, 1, MCW);
        }

    }
    else {
        // I am Chef
        srand(time(NULL) + rank);
        while (1) {
            MPI_Iprobe(0, MPI_ANY_TAG, MCW, &flag, &status);
            if (flag) {
                cout << rank << " You youngins just don't to work anymore." << endl;
                MPI_Recv(&data, 1, MPI_INT, 0, 1, MCW, MPI_STATUS_IGNORE);
                break;
            }
            else {
                Sleep((rand() % 5 + 1) * 1000);
                MPI_Send(&rank, 1, MPI_INT, 0, 1, MCW);
                cout << "Chef: I Hungy." << endl;
            }
        }
    }
    MPI_Finalize();
    return 0;
}