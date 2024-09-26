#include <iostream>
#include <thread>
#include <vector>
#include <unistd.h>
#include <string>
#include <mutex>
#include <queue>
#include <fstream>
#include <condition_variable>

using namespace std;

mutex buffer_mtx;
condition_variable cv;
queue<int> buffer;

void print_consumer(int element)
{
    size_t tid = hash<thread::id>{}(this_thread::get_id());
    printf("[%016zX] %d\n", tid, element);
}

void consumer()
{
    while (true)
    {

        //=====================================================================
        // BEGIN SKILL ASSESSMENT FILL-IN PORTION
        //=====================================================================
        
        //TODO: wait for the buffer to have an element in it using cv
        std::unique_lock<std::mutex> l{buffer_mtx};
        cv.wait(l, []{ return buffer.size() > 0; });

        //TODO: pop the front item out of the queue
        int cur = buffer.front();
        buffer.pop();
        //if the value is -1, exit the loop;
        if (cur == -1) break;
        
        //else, use print_consumer to print it
        print_consumer(cur);
    
        l.unlock();
        cv.notify_one();

        //=====================================================================
        // END SKILL ASSESSMENT FILL-IN PORTION
        //=====================================================================

        usleep(1);
    }
}

int main(int argc, char **argv)
{

    // args
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <number of consumers>\n", argv[0]);
        return EXIT_FAILURE;
    }
    int n_consumers = atoi(argv[1]);
    fprintf(stderr, "Consumers: %d\n", n_consumers);

    // start consumer threads
    vector<thread> consumer_threads;
    for (int i = 0; i < n_consumers; ++i)
    {
        consumer_threads.push_back(thread(consumer));
        fprintf(stderr, " Consumer %d created\n", i + 1);
    }

    // run the producer
    while (true)
    {
        // get input
        string line;
        if (!getline(cin, line) || line == "")
        {
            break;
        }
        int val = stoi(line);
        if (val == 0)
        {
            fprintf(stderr, "Detected 0 in input, what happened?\n");
            break;
        }
        // send to consumers
        unique_lock<mutex> lock(buffer_mtx);
        buffer.push(val);
        lock.unlock();
        cv.notify_one();
    }

    // send exit signal to consumers
    unique_lock<mutex> lock(buffer_mtx);
    for (int i = 0; i < n_consumers; ++i)
    {
        buffer.push(-1);
    }
    lock.unlock();
    cv.notify_all();

    // wait for consumers to finish
    for (int i = 0; i < n_consumers; ++i)
    {
        consumer_threads[i].join();
        fprintf(stderr, " Consumer %d exited\n", i + 1);
    }

    // done
    fprintf(stderr, "Bye!\n");
    return 0;
}