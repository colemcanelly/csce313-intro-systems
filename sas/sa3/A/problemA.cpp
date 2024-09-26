#include <iostream>
#include <thread>
#include <vector>
#include <fstream>
#include <mutex>

using namespace std;

vector<int> vec;
int result_sum = 0;
mutex mtx;

void calculate_sum(int start, int len){
    size_t tid = hash<thread::id>{}(this_thread::get_id());
    fprintf(stderr, "[%016zX] %d numbers\n", tid, len);
    int sum = 0;
    for (int i = 0; i < len; ++i){
        sum += vec[start + i];
    }
    mtx.lock();
    result_sum += sum;
    mtx.unlock();
}

void create_threads(int num_threads, int num_elements){

    printf("Threads to create: %d\n", num_threads);
    printf("Elements to sum: %d\n", num_elements);

    //=====================================================================
    //BEGIN SKILL ASSESSMENT FILL-IN PORTION
    //=====================================================================
    
    //TOOD: you'll need some way to store the threads
    std::vector<std::thread> t_vec;
    int length = num_elements / num_threads;

    //TODO: create the threads
    for ( int i = 0; i < num_elements; i += length )
    {
        t_vec.push_back(std::thread(calculate_sum, i, length));
    }
    //have them run the calculate_sum function
    //each thread should sum an equal portion of the entire num_elements in the vector

    //TODO: wait for all of the threads to finish
    for ( std::thread& t : t_vec )
    {
        t.join();
    }

    //=====================================================================
    //END SKILL ASSESSMENT FILL-IN PORTION
    //=====================================================================

    printf("Result sum: %d\n", result_sum);
}


int main(int argc, char* argv[]){
    if (argc != 3){
        printf("Usage: %s <data file> <number of threads>\n", argv[0]);
        return EXIT_FAILURE;
    }
    //load data
    ifstream ifs(argv[1]);
    string line;
    while(getline(ifs, line)){
        vec.push_back(stoi(line));
    }
    ifs.close();

    create_threads(stoi(argv[2]), vec.size());

    return 0;
}