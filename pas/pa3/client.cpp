#include <fstream>
#include <iostream>
#include <thread>
#include <sys/time.h>
#include <sys/wait.h>

#include "BoundedBuffer.h"
#include "common.h"
#include "Histogram.h"
#include "HistogramCollection.h"
#include "FIFORequestChannel.h"

#include "client.h"

// ecgno to use for datamsgs
#define ECGNO 1

using namespace std;


void patient_thread_function ( BoundedBuffer* p_reqbuf, int __patient, int __n_reqs )
{
    double time_offset = 0.004;
    datamsg d_msg{__patient, 0.000, ECGNO};
    for (int i = 0; i < __n_reqs; ++i)
    {
        p_reqbuf->push((char*)&d_msg, sizeof(datamsg));
        d_msg.seconds += time_offset;
    }
}

void file_thread_function ( BoundedBuffer* p_reqbuf, string __fname, int64_t __fsize, int __bufcap )
{
    FILE* file_ptr = fopen(("received/" + __fname).c_str(), "w+");
    fseek(file_ptr, __fsize, SEEK_SET);
    fclose(file_ptr);
    int numReqs = __fsize / __bufcap;
	for (int i = 0; i < numReqs; ++i)
    {
        file_thread_helper(p_reqbuf, __fname, i * __bufcap, __bufcap);
    }
    int remaining = __fsize % __bufcap;
	if (remaining) file_thread_helper(p_reqbuf, __fname, __fsize - remaining, remaining);
}

// hello cole :D

void worker_thread_function ( BoundedBuffer* p_reqbuf, BoundedBuffer* p_resbuf, FIFORequestChannel* p_channel, int __bufcap )
{
    char* buffer = new char[__bufcap];
    while (true)
    {
        p_reqbuf->pop(buffer, __bufcap);
        switch (*((MESSAGE_TYPE*)buffer))
        {
        case DATA_MSG:
            worker_thread_data_helper(buffer, p_resbuf, p_channel);
            break;
        case FILE_MSG:
            worker_thread_file_helper(buffer, p_channel, __bufcap);
            break;
        case QUIT_MSG:
            p_channel->cwrite(((char*)((MESSAGE_TYPE*)buffer)), sizeof(MESSAGE_TYPE));
            delete[] buffer;
            delete p_channel;
            return;
        default:
            break;
        }
    }
    delete[] buffer;
}

void histogram_thread_function ( BoundedBuffer* p_resbuf, HistogramCollection* p_histo_coll, int __bufcap )
{
    char* buffer = new char[__bufcap];
    while (true)
    {
        p_resbuf->pop(buffer, __bufcap);
        std::pair<int, double>* p_response = (std::pair<int, double>*)buffer;
        if ((p_response->first == -1) && (p_response->second == -1.0)) break;
        p_histo_coll->update(p_response->first, p_response->second);
    }
    delete[] buffer;
}


int main (int argc, char* argv[]) {
    int n = 1000;	// default number of requests per "patient"
    int p = 10;		// number of patients [1,15]
    int w = 100;	// default number of worker threads
	int h = 20;		// default number of histogram threads
    int b = 20;		// default capacity of the request buffer (should be changed)
	int m = MAX_MESSAGE;	// default capacity of the message buffer
	string f = "";	// name of file to be transferred
    
    // read arguments
    int opt;
	while ((opt = getopt(argc, argv, "n:p:w:h:b:m:f:")) != -1) {
		switch (opt) {
			case 'n':
				n = atoi(optarg);
                break;
			case 'p':
				p = atoi(optarg);
                break;
			case 'w':
				w = atoi(optarg);
                break;
			case 'h':
				h = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
                break;
			case 'm':
				m = atoi(optarg);
                break;
			case 'f':
				f = optarg;
                break;
		}
	}
    
	// fork and exec the server
    int pid = fork();
    if (pid == 0) {
        execl("./server", "./server", "-m", (char*) to_string(m).c_str(), nullptr);
    }
    
	// initialize overhead (including the control channel)
	FIFORequestChannel* chan = new FIFORequestChannel("control", FIFORequestChannel::CLIENT_SIDE);
    BoundedBuffer request_buffer(b);
    BoundedBuffer response_buffer(b);
	HistogramCollection hc;

    // making histograms and adding to collection
    for (int i = 0; i < p; i++) {
        Histogram* h = new Histogram(10, -2.0, 2.0);
        hc.add(h);
    }
	
	// record start time
    struct timeval start, end;
    gettimeofday(&start, 0);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /* create all threads here */

    // PRODUCES REQUESTS :: patient and file threads
    vector<thread*> patient_threads;
    for (int patient = 1; patient <= p; ++patient)
    {
        patient_threads.push_back(new thread{patient_thread_function, &request_buffer, patient, n});
    }

    thread file_thread;
    if (f != "") {
        file_thread_helper(chan, f);
        int64_t fsize;
        chan->cread(&fsize, sizeof(int64_t));
        file_thread = thread{file_thread_function, &request_buffer, f, fsize, m};
    }

    // CONSUMES REQUESTs & PRODUCES RESPONSES :: worker threads
    vector<worker*> worker_threads;
    for (int i = 1; i <= w; ++i)
    {
        worker_threads.push_back(new worker{&request_buffer, &response_buffer, chan, m});
    }

    // CONSUMES RESPONSES :: histogram threads
    vector<thread*> histogram_threads;
    for (int i = 0; i < h; ++i)
    {
        histogram_threads.push_back(new thread{histogram_thread_function, &response_buffer, &hc, m});
    }

    // All threads created
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* join all threads here */

    // Finish patients
    for (thread*& t_patient : patient_threads)
    {
        t_patient->join();
        delete t_patient;
    }

    // Finish File request
    if (f != "") file_thread.join();

    // Finish workers
    for (int i = 0; i < w; ++i)
    {
        MESSAGE_TYPE q = QUIT_MSG;
        request_buffer.push((char *) &q, sizeof (MESSAGE_TYPE));
    }
    for (worker*& worker : worker_threads)
    {
        delete worker;
    }

    // Finish historgrams
    std::pair<int, double> quit{-1, -1.0}; // might have to create inside loop
    for (int i = 0; i < h; ++i)
    {
        response_buffer.push((char*)&quit, sizeof(quit));
    }
    for (thread*& gram : histogram_threads)
    {
        gram->join();
        delete gram;
    }
    // All threads joined
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// record end time
    gettimeofday(&end, 0);

    // print the results
	if (f == "") {
		hc.print();
	}
    int secs = ((1e6*end.tv_sec - 1e6*start.tv_sec) + (end.tv_usec - start.tv_usec)) / ((int) 1e6);
    int usecs = (int) ((1e6*end.tv_sec - 1e6*start.tv_sec) + (end.tv_usec - start.tv_usec)) % ((int) 1e6);
    cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;

	// quit and close control channel
    MESSAGE_TYPE q = QUIT_MSG;
    chan->cwrite ((char *) &q, sizeof (MESSAGE_TYPE));
    cout << "All Done!" << endl;
    delete chan;

	// wait for server to exit
	wait(nullptr);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// File Thread Helper //////////////////////////////////////////////////

void file_thread_helper ( void* p_dest, string& __fname, int64_t __offset, int __length )
{
    filemsg f_msg(__offset, __length);

	const int msgLen = sizeof(filemsg) + (__fname.size() + 1);
	char* buffer = new char[msgLen]; 
	memcpy(buffer, &f_msg, sizeof(filemsg));
	strcpy(buffer + sizeof(filemsg), __fname.c_str());

    if (!(__offset || __length)) {       // FIFORequestChannel
        ((FIFORequestChannel*)p_dest)->cwrite(buffer, msgLen);
    } else {                        // Bounded buffer
	    ((BoundedBuffer*)p_dest)->push(buffer, msgLen);
    }
    delete[] buffer;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// Worker Thread Helpers ///////////////////////////////////////////////

void worker_thread_data_helper ( char* __d_msg, BoundedBuffer* p_resbuf, FIFORequestChannel* p_channel )
{
    p_channel->cwrite(__d_msg, sizeof(datamsg));
	double reply;
	p_channel->cread(&reply, sizeof(double));
    std::pair<int, double> response{((datamsg*)__d_msg)->person, reply};
    p_resbuf->push((char*)&response, sizeof(response));
}


void worker_thread_file_helper ( char* __f_msg, FIFORequestChannel* p_channel, int __bufcap )
{
    string fname = __f_msg + sizeof(filemsg);
    int msgLen = sizeof(filemsg) + (fname.size() + 1);

    if (((filemsg*)__f_msg)->length < __bufcap) printf("breakpoint");
    
    char* buffer = new char[__bufcap];
    p_channel->cwrite(__f_msg, msgLen);
    p_channel->cread(buffer, __bufcap);

    FILE* p_file = fopen(("received/" + fname).c_str(), "r+");
    fseek(p_file, ((filemsg*)__f_msg)->offset, SEEK_SET);
    fwrite(buffer, 1, ((filemsg*)__f_msg)->length, p_file);
    fclose(p_file);
    delete[] buffer;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////