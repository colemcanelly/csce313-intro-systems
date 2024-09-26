#include <cstdio>

#include <unistd.h>

#include <condition_variable>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

using namespace std;

//helper function to print messages for you
void print(int id, int message) {
    size_t tid = hash<thread::id>{}(this_thread::get_id());
    fprintf(stderr, "[%016zX]\n", tid); //print thread id to stderr
    printf("%c: %d\n", 'A'+ id, message); //print message to stdout
}

//encapsulates all of the shared state for the threads to use
struct ThreadInfo {
    //task information
    int nThreads; //the total number of threads (A, B, C, ...)
    int nLinesPerThread; //the number of lines of text to print in each thread
    //synchronization
    mutex m; //you can use this mutex to protect the shared state
    condition_variable* cvs; //you have access to 1 CV per thread; you can use these however you want

    //=====================================================================
    //BEGIN SKILL ASSESSMENT FILL-IN PORTION 1
    //=====================================================================

    //shared state; you can add whatever you want here
    int running = 0;

    //=====================================================================
    //END SKILL ASSESSMENT FILL-IN PORTION 1
    //=====================================================================
};

void worker(ThreadInfo* info, int id) {
    //=====================================================================
    //BEGIN SKILL ASSESSMENT FILL-IN PORTION 2
    //=====================================================================

    /*
    TODO: use the synchronization variables available in the ThreadInfo object,
    including any variables you add in fill-in portion 1,
    to ensure that the threads execute in order without interleaving.
    For example, if there are three threads (A, B, and C),
    the execution order should be something like:
    A1, A2, A3, ..., An, B1, B2, B3, ..., Bn, C1, C2, C3, ..., Cn
    (where n is a placeholder for the number of lines printed in each thread).
    */
    std::unique_lock<std::mutex> l{info->m};
    info->cvs[0].wait(l, [info, id]{ return info->running == id; });
    
    //unsynchronized starter
    //you can change this however you want, including erasing it completely
    for(int i = 1; i <= info->nLinesPerThread; i++) {
        print(id, i); //use this function! don't print manually!
    }

    (info->running)++;        // Increment the thread that should run, if A ==> B
    l.unlock();
    info->cvs[0].notify_all();

    //=====================================================================
    //END SKILL ASSESSMENT FILL-IN PORTION 2
    //=====================================================================
}

int main(int argc, char** argv){

    if(argc < 3) {
        printf("Usage: %s <number of threads> <number of lines>\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("Hi!\n");

    int nThreads = atoi(argv[1]);
    int nLines = atoi(argv[2]);

    if(nThreads <= 0 || nThreads > 25) {
        printf("Invalid thread count: %d\n", nThreads);
        return EXIT_FAILURE;
    }

    if(nLines <= 0) {
        printf("Invalid line count: %d\n", nLines);
        return EXIT_FAILURE;
    }

    printf("Threads = %d, lines = %d\n", nThreads, nLines);
    
    //initialize task info
    ThreadInfo info{};
    info.nThreads = nThreads;
    info.nLinesPerThread = nLines;
    //create condition variables
    info.cvs = new condition_variable[nThreads];
    //create random order for threads to start
    vector<int> threadIds;
    for(int i = 0; i < nThreads; i++) {
        threadIds.push_back(i);
    }
    std::default_random_engine rng;
    //set random seed
    struct timespec tp;
    if(clock_gettime(CLOCK_MONOTONIC, &tp) < 0) {
        perror("clock_gettime failed");
        return EXIT_FAILURE;
    }
    rng.seed(tp.tv_nsec);
    // std::shuffle(threadIds.begin(), threadIds.end(), rng); //std::shuffle is terrible for small arrays, we're doing it by hand
    for(int i = 0; i < nThreads - 1; i++) {
        std::uniform_int_distribution dist(i + 1, nThreads - 1);
        std::swap(threadIds[i], threadIds[dist(rng)]);
    }
    //start threads
    vector<thread> threads;
    for(int i : threadIds) {
        threads.push_back(thread(worker, &info, i));
    }
    //join threads
    for(int i = 0; i < nThreads; i++) {
        threads[i].join();
    }
    //delete condition variables
    delete[] info.cvs;
    
    printf("Bye!\n");

    return 0;
}