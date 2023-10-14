#include <iostream>
#include <mpi.h>
#include <io.h>
#include <vector>
#include <Windows.h>
#define MCW MPI_COMM_WORLD


#include <chrono>
using namespace std::chrono;

using namespace std;



int main(int argc, char** argv) {
    int rank, size;
    srand(1969);
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    MPI_Request request;
    MPI_Status status;
    int flag = 0;

    vector<int> work_queue;
    int work_queue_max = 16;
    
    int work = 0;
    int work_max = 0;
    int total_tasks = 0;

    int random_location = rank;

    int tasks_to_generate = 0;

    int tasks_generated = 0;
    int tasks_generated_max = 0;

    int WHITE = 0;
    int BLACK = 1;
    int color = WHITE; //0 = White, 1 = Black
    int token_color = WHITE;
    bool has_token = true;
    bool finalized = false;

    int idle_ticks = 0;

    int recv_process = rank - 1;
    auto start = high_resolution_clock::now();

    if (!rank) {
        
        recv_process = size - 1;
        tasks_generated_max = rand() % 1024 + 1024;
        cout << "# " << tasks_generated_max << endl;
        for (int i = 1; i < size; i++) {
            MPI_Send(&tasks_generated_max, 1, MPI_INT, i, 4, MCW);
        }
    }
    else {
         MPI_Recv(&tasks_generated_max, 1, MPI_INT, 0, 4, MCW, MPI_STATUS_IGNORE);
    }

    while (1) {

        if (!rank) {
            MPI_Iprobe(MPI_ANY_SOURCE, 6, MCW, &flag, &status);
            if (flag) {
                while (flag) {
                    tasks_to_generate = 0;
                    MPI_Recv(&tasks_to_generate, 1, MPI_INT, MPI_ANY_SOURCE, 6, MCW, MPI_STATUS_IGNORE);
                    total_tasks += tasks_to_generate;
                    MPI_Iprobe(MPI_ANY_SOURCE, 6, MCW, &flag, &status);
                }
                for (int i = 0; i < size; i++) {
                    if (i % 2 == 1) {
                        MPI_Send(&total_tasks, 1, MPI_INT, i, 5, MCW);
                    }
                }
            }
        }
        MPI_Iprobe(0, 5, MCW, &flag, &status);
        while (flag) {
            MPI_Recv(&total_tasks, 1, MPI_INT, 0, 5, MCW, MPI_STATUS_IGNORE);
            MPI_Iprobe(0, 5, MCW, &flag, &status);
        }

        MPI_Iprobe(0, 3, MCW, &flag, &status);
        if (flag) {
            MPI_Recv(&work, 1, MPI_INT, 0, 3, MCW, MPI_STATUS_IGNORE);
            break;
        }


        MPI_Iprobe(MPI_ANY_SOURCE, 1, MCW, &flag, &status);
        while (flag) {
            MPI_Recv(&work, 1, MPI_INT, MPI_ANY_SOURCE, 1, MCW, MPI_STATUS_IGNORE);
            //cout << rank << " added work: " << work << endl;
            work_queue.push_back(work);
            MPI_Iprobe(MPI_ANY_SOURCE, 1, MCW, &flag, &status);
        }

        if (work_queue.size() > work_queue_max) {
            //cout << rank << " too much work " << endl;
            for (int i = 0; i < 2; i++) { /* execute twice*/
                random_location = rand() % size;
                while (random_location == rank) {
                    random_location = rand() % size;
                }
                work = work_queue.back();
                //cout << rank << " sent " << work << " to " << random_location << endl;
                MPI_Send(&work, 1, MPI_INT, random_location, 1, MCW);
                work_queue.pop_back();

                if (random_location < rank) { /* turns process black if it sends backward */
                    //cout << "BLACK" << endl;
                    color = BLACK;
                }
            }
        }

        if (work_queue.size()) {
            work = work_queue.back();
            work_max = work * work;

            //cout << rank << " doing work " << work << endl;

            for (int i = work; i < work_max; i++) {
                work++;
            }
            work_queue.pop_back();
        }

        if (rank % 2 == 1) {
            //cout << total_tasks << endl;
            if (total_tasks < tasks_generated_max) {
                tasks_to_generate = rand() % 3 + 1;
                MPI_Send(&tasks_to_generate, 1, MPI_INT, 0, 6, MCW);

                for (int i = 0; i < tasks_to_generate; i++) {
                    if (total_tasks < tasks_generated_max) {
                        work = rand() % 1024;
                        //cout << rank << " creating work " << work << endl;
                        work_queue.push_back(work);
                        total_tasks++;
                    }
                    else {
                        signed int x = -1 * work_queue.size();
                        work_queue.clear();
                        MPI_Send(&x, 1, MPI_INT, 0, 6, MCW);
                    }
                }
            }
            else {
                signed int x = -1 * work_queue.size();
                work_queue.clear();
                MPI_Send(&x, 1, MPI_INT, 0, 6, MCW);
            }
        }

        if (rank == 0 && work_queue.size() == 0 && has_token) {
            cout << "****************************** 0 sending " << (rank + 1) % size << " a " << token_color << endl;
            MPI_Send(&token_color, 2, MPI_INT, (rank + 1) % size, 2, MCW);
            has_token = false;
        }

        if (work_queue.size() == 0) {
            idle_ticks++;
            MPI_Iprobe(recv_process, 2, MCW, &flag, &status);
            if (flag) {
                MPI_Recv(&token_color, 2, MPI_INT, recv_process, 2, MCW, MPI_STATUS_IGNORE);

                cout << " ************************************** " << rank << " Recieved from " << recv_process << " a " << token_color << endl;

                if (color == BLACK) {
                    token_color = BLACK;
                }

                if (token_color == BLACK) {
                    token_color = BLACK;
                    color = WHITE;
                }

                if (!rank && token_color == WHITE) {
                    cout << "Done" << endl;
                    finalized = true;
                    break;
                }

                if (!rank) {
                    token_color = WHITE;
                }

                MPI_Send(&token_color, 2, MPI_INT, (rank + 1) % size, 2, MCW);
            }
        }

        
        if (!rank && token_color == WHITE && finalized) {
            break;
        }
        
    }

    if (!rank) {
        auto stop = high_resolution_clock::now();
        for (int i = 1; i < size; ++i) {
            work = -1;
            MPI_Send(&work, 1, MPI_INT, i, 3, MCW);
        }
        cout << "# " << total_tasks << endl;
        auto duration = duration_cast<microseconds>(stop - start);
        cout << "Duration " << duration.count() << endl;
    }
    cout << rank << " " << work_queue.size()<< " Idle Ticks: "<<idle_ticks << endl;
    MPI_Finalize();
    return 0;
}