#include "RoundRobin.h"

/*
This is a constructor for RoundRobin Scheduler, you should use the extractProcessInfo function first
to load process information to process_info and then sort process by arrival time;

Also initialize time_quantum
*/
RoundRobin::RoundRobin(string file, int time_quantum) : processVec{}, waitQueue{} {
	set_time_quantum(time_quantum);
	extractProcessInfo(file);
}

// Schedule tasks based on RoundRobin Rule
// the jobs are put in the order the arrived
// Make sure you print out the information like we put in the document
void RoundRobin::schedule_tasks() {
	int time = 0, burst = 0;
	const int incr = 1;
	while (!(processVec.empty() && waitQueue.empty()))
	{
		// check for new arrivals
		while (!processVec.empty()) {
			if (processVec.front()->get_arrival_time() <= time) {
				waitQueue.push(processVec.front());
				processVec.pop();
			}
			break;
		}

		// NO OP
		if (waitQueue.empty()) {
			print(time, -1, false);
		}
		else {
			shared_ptr<Process> curr = waitQueue.front();
			// check if complete : continue
			if (curr->is_Completed()) {
				print(time, curr->getPid(), true);
				waitQueue.pop();
				burst = 0;
				continue;
			}

			// check time, preempt if necessary
			if (burst == time_quantum) {
				waitQueue.push(curr);
				waitQueue.pop();
				burst = 0;
				continue;
			}

			// run next process
			print(time, curr->getPid(), curr->is_Completed());
			curr->Run(incr);
		}
		// increment system time
		time += incr;
		burst += incr;

	}
}


/*************************** 
ALL FUNCTIONS UNDER THIS LINE ARE COMPLETED FOR YOU
You can modify them if you'd like, though :)
***************************/


// Default constructor
RoundRobin::RoundRobin() {
	time_quantum = 0;
}

// Time quantum setter
void RoundRobin::set_time_quantum(int quantum) {
	this->time_quantum = quantum;
}

// Time quantum getter
int RoundRobin::get_time_quantum() {
	return time_quantum;
}

// Print function for outputting system time as part of the schedule tasks function
void RoundRobin::print(int system_time, int pid, bool isComplete){
	string s_pid = pid == -1 ? "NOP" : to_string(pid);
	cout << "System Time [" << system_time << "].........Process[PID=" << s_pid << "] ";
	if (isComplete)
		cout << "finished its job!" << endl;
	else
		cout << "is Running" << endl;
}

// Read a process file to extract process information
// All content goes to proces_info vector
void RoundRobin::extractProcessInfo(string file){
	// open file
	ifstream processFile (file);
	if (!processFile.is_open()) {
		perror("could not open file");
		exit(1);
	}

	// read contents and populate process_info vector
	string curr_line, temp_num;
	int curr_pid, curr_arrival_time, curr_burst_time;
	while (getline(processFile, curr_line)) {
		// use string stream to seperate by comma
		stringstream ss(curr_line);
		getline(ss, temp_num, ',');
		curr_pid = stoi(temp_num);
		getline(ss, temp_num, ',');
		curr_arrival_time = stoi(temp_num);
		getline(ss, temp_num, ',');
		curr_burst_time = stoi(temp_num);
		shared_ptr<Process> p(new Process(curr_pid, curr_arrival_time, curr_burst_time));

		processVec.push(p);
	}

	// close file
	processFile.close();
}