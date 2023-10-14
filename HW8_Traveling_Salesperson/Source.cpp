#include <mpi.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <fstream>
#include <cmath> 
#include <sstream>
#include <algorithm>

using namespace std;
#define MCW MPI_COMM_WORLD

vector<vector<int>> readCities() {
    std::string line;
    int word;

    std::ifstream inFile("checkpoint8000.txt");

    std::vector<std::vector<int>> cities;

    if (inFile) {
        while (getline(inFile, line, '\n')) {
            std::vector<int> tempVec;

            std::istringstream ss(line);

            while (ss >> word) {
                tempVec.push_back(word);
            }
            cities.emplace_back(tempVec);
        }
    }

    else {
        std::cout << "file cannot be opened" << std::endl;
    }

    inFile.close();

    return cities;
}

void writeCities(vector<vector<int>> cities, int iteration) {
    ofstream myfile;
    stringstream ss;
    ss <<"checkpoint"<< iteration << ".txt";
    myfile.open(ss.str());
    for (int i = 0; i < cities.size(); i++) {
        myfile << cities[i][0] << " " << cities[i][1] << "\n";
    }
    myfile.close();
    ss.str("");
    return;
}

void writeData(int fitness, int iteration) {
    ofstream myfile;
    myfile.open("data.txt", std::ios_base::app);
    myfile << iteration << " " << fitness << "\n" << endl;
    myfile.close();
    return;
}

double distanceCalculate(double x1, double y1, double x2, double y2)
{
    double x = x1 - x2; //calculating number to square in next step
    double y = y1 - y2;
    double dist;

    dist = pow(x, 2) + pow(y, 2);       //calculating Euclidean distance
    dist = sqrt(dist);

    return dist;
}

double fitness(std::vector<std::vector<int>> cities) {
    double total_distance = 0.0;
    for (int i = 0; i < cities.size(); i++) {
        int j = i + 1;
        if (j < cities.size()) {
            total_distance += abs(distanceCalculate(cities[i][0], cities[i][1], cities[j][0], cities[j][1]));
        }
    }
    return total_distance;
}

int findIndex(int x_val, int y_val, std::vector<std::vector<int>> cities) {
    for (int i = 0; i < cities.size(); i++) {
        if (cities[i][0] == x_val and cities[i][1] == y_val) {
            return i;
        }
    }
}

std::vector<std::vector<int>> partialMatchedCrossover(std::vector<std::vector<int>> cities1, std::vector<std::vector<int>> cities2) {
    int array_size = cities1.size();
    int start = rand() % array_size + 0;
    int end = rand() % array_size + 0;

    int steps = end - start;
    if (start > end) {
        steps = array_size - (start - end);
    }

    for (int i = 0; i < steps; i++) {
        int i_i = (i + start) % array_size;
        int c2_found = findIndex(cities1[i][0], cities1[i][1], cities2);

        std::vector<int> tempVec = cities2[c2_found];
        cities2[c2_found] = cities2[i_i];
        cities2[i_i] = tempVec;
    }
    return cities2;
}

vector<vector<int>> mutation(std::vector<std::vector<int>> cities) {
    int array_size = cities.size();
    int start = rand() % array_size + 0;
    int end = rand() % array_size + 0;

    int steps = end - start;
    if (start > end) {
        steps = array_size - (start - end);
    }

    for (int i = 0; i < steps/2; i++) {
        int upper_i = (i + start) % array_size;
        int lower_i = end - i;

        if (lower_i < 0) {
            lower_i = array_size + lower_i;
        }
        std::vector<int> tempVec = cities[lower_i];
        cities[lower_i] = cities[upper_i];
        cities[upper_i] = tempVec;
    }
    return cities;
}


bool comp(pair<int, int> A, pair<int, int> B) {
    // Else compare values
    return A.second < B.second;
}

int main(int argc, char** argv) {
    int rank, size;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);

    MPI_Request request;
    MPI_Status status;
    int flag = 0;
    int token;
    int cities_tag = 0;

    int next_rank = (rank + 1) % size;
    int prev_rank = (rank - 1) > -1 ? rank - 1 : size-1;

    vector<vector<int>> cities;
    vector<vector<int>> temp_cities;
    vector<vector<vector<int>>> parents;
    vector<vector<vector<int>>> children;
    vector<pair<int, int> > fitnesses;
    int children_count = 30;
    int parents_count = 5;
    int to_send = 10;

    int rand_parent_1 = 0;
    int rand_parent_2 = 2;

    int iteration = 0;
    int mutation_rate = 100;
    int checkpoint_rate = 1000;
    int data_rate = 100;
    int communicate_rate = 777;
    int temperature = 5;
    int temperature_rate = 100;

    int print_rate = 100;

    int work;

    int codes[100];

    srand(time(NULL) + rank);

    // TODO temperature
    //read cities as index rather than coord pairs
    cout << "Rank " << rank << " Prev " << prev_rank << " next " << next_rank << endl;
	cities = readCities();
    for (int i = 0; i < parents_count; i++) {
        temp_cities = cities;
        for (int j = 0; j < 100; j++) {
            temp_cities = mutation(temp_cities); // many mutations
        }
        parents.push_back(temp_cities);
    }

    while (1) {

        for (int i = 0; i < children_count; i++) {
            rand_parent_1 = rand() % parents.size() + 0;
            rand_parent_2 = rand() % parents.size() + 0;
            while (rand_parent_2 == rand_parent_1) {
                rand_parent_2 = rand() % parents.size() + 0;
            }

            vector<vector<int>> child = partialMatchedCrossover(parents[rand_parent_1], parents[rand_parent_2]);

            if (iteration % mutation_rate == 0) {
                while((rand() % 10 + 0) == 0) {
                    child = mutation(child);
                    child = mutation(child);
                    child = mutation(child);
                }
            }

            children.push_back(child);
        }

        for (int i = 0; i < children.size(); i++) {
            fitnesses.push_back(make_pair(i, fitness(children[i])));
        }

        sort(fitnesses.begin(), fitnesses.end(), comp);

        /////////////////////////////////////////////////////////

        if (iteration % communicate_rate == 0 and iteration) {
            for (int i = 0; i < to_send; i++) {
                
                for (int j = 0; j < children[fitnesses[i].first].size(); j++) {
                    codes[j] = findIndex(children[fitnesses[i].first][j][0], children[fitnesses[i].first][j][1], cities);
                }
                MPI_Send(&codes, 100, MPI_INT, next_rank, cities_tag, MCW);
            }
        }
        //cout << "TT " << children[0][0][0] << endl;
        MPI_Iprobe(prev_rank, cities_tag, MCW, &flag, &status);
        if (flag) {
            while (flag) {
                MPI_Recv(&codes, 100, MPI_INT, prev_rank, cities_tag, MCW, MPI_STATUS_IGNORE);
                vector<vector<int>> temp;
                for (int j = 0; j < children[0].size(); j++) {
                    temp.push_back(cities[codes[j]]);
                }
                children.push_back(temp);
                MPI_Iprobe(prev_rank, cities_tag, MCW, &flag, &status);
            }
            // resort based on migratory children
            fitnesses.clear();
            for (int i = 0; i < children.size(); i++) {
                fitnesses.push_back(make_pair(i, fitness(children[i])));
            }

            sort(fitnesses.begin(), fitnesses.end(), comp);

        }
        /////////////////////////////////////////////////////////////////
        vector<vector<int>> temp;
        if (fitness(parents[0]) < fitnesses[0].second){
            temp = parents[0];
        }
        parents.clear();
        for (int i = 0; i < parents_count; i++) {
            if (i == 0 and temp.size()) {
                parents.push_back(temp);
            }
            else {
                if (rand() % temperature_rate < temperature) {
                    parents.push_back(children[fitnesses[(rand() % children.size() + 0)].first]);
                }
                else {
                    parents.push_back(children[fitnesses[i].first]);
                }
            }
        }
        if (iteration % print_rate == 0 and iteration) {
            cout <<"Rank "<< rank << " | " << iteration << " Best " << fitnesses[0].second << endl;
        }

        if (!rank) {
            if (iteration % checkpoint_rate == 0) {
                writeCities(children[fitnesses[0].first], iteration);
            }
            if (iteration % data_rate == 0) {
                writeData(fitnesses[0].second, iteration);
            }
        }
        children.clear();
        fitnesses.clear();

        if (iteration % 20 == 0 and iteration) {
            if (temperature > 0) {
                temperature--;
            }
        }

        iteration++;
            
    }
    
    MPI_Finalize();
    return 0;
}