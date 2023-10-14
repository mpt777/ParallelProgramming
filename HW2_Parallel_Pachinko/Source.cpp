#include <iostream>
#include <mpi.h>
#include <windows.h>
#include <fstream>
#define MCW MPI_COMM_WORLD
using namespace std;

int sendDirection(int layer) {

    int rank, size;
    int dest;
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    bool valid = false;
        //cout << rank << ": " << "received the potato." << endl;
    dest = rank;
    int direction = rand() % 2;
    if (direction == 0) dest--; else dest++;
    layer--;
        //cout << layer << ": " << "layer." << endl;
    if (dest >= 0 && dest < size) {

    }
    else {
        //bounce back to center
        if (dest < 0) dest += 2;
        if (dest >= size) dest -= 2;
    }

    MPI_Send(&layer, 1, MPI_INT, dest, 0, MCW);

    return 0;
}

int newIteration(int layer, int new_game_column) {
    //cout << layer << "New iteration" << endl;

    int rank, size;
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);
    int dest;
    layer--;
    dest = rand() % size;

    //if new_game_column is valid, use it. Else, randomly pick in the while loop
    if (new_game_column >= 0 && new_game_column < size) dest = new_game_column;

    while (dest == rank) {
        dest = rand() % size;
    }
    //cout << dest << endl;
    cout << rank << ": " << "boom!" << endl;

    // log data
    string filename("data.txt");
    fstream file;
    file.open(filename, std::ios_base::app | std::ios_base::in);
    if (file.is_open())
        file << rank << endl;

    MPI_Send(&layer, 1, MPI_INT, dest, 0, MCW);
    return 0;
}

int endSimulation(int layer) {

    int rank, size;
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    layer--;
    for (int i = 0; i < size; ++i) {
        if (i != rank) {
            MPI_Send(&layer, 1, MPI_INT, i, 0, MCW);
        }
    }
    return 0;
}


int main(int argc, char** argv) {
    int rank, size;
    int layer;
    int layers_per_game = atoi(argv[1]);
    int dest;
    int iterations = atoi(argv[2]);
    int new_game_column = -1;

    remove("data.txt");
    if (argc > 3) new_game_column = atoi(argv[3]);
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    if (!rank) {
        layer = (layers_per_game * iterations) + (layers_per_game-1);
        newIteration(layer, new_game_column);
    }
    while (1) {
        MPI_Recv(&layer, 1, MPI_INT, MPI_ANY_SOURCE, 0, MCW, MPI_STATUS_IGNORE);
        //Sleep(10);
        if (layer <= 0) {
            endSimulation(layer);
            break;
        }
        else if (layer % layers_per_game == 0) {
            //cout << "new: "<< layer << endl;
            newIteration(layer, new_game_column);
        }
        else {
            sendDirection(layer);
        }
    }
    cout << rank << ": " << "done." << endl;
    MPI_Finalize();
    return 0;
}
