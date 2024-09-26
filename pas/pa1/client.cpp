/*
	Original author of the starter code
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 2/8/20
	
	Please include your Name, UIN, and the date below
	Name: Cole McAnelly
	UIN: 630003358
	Date: 9/22/2022
*/
#include "common.h"
#include "FIFORequestChannel.h"
#include <sys/wait.h>

using namespace std;


double request_single_datapoint(FIFORequestChannel* channel, int patientID, double time, int ecg) {
	datamsg msg{patientID, time, ecg};

	char buffer[MAX_MESSAGE];
	memcpy(buffer, &msg, sizeof(datamsg));

	channel->cwrite(buffer, sizeof(datamsg));
	double reply;
	channel->cread(&reply, sizeof(double));

	return reply;
}

void request_n_datapoints(FIFORequestChannel* channel, int patientID, int dataCount = 1000) {
	double reply_ecg1, reply_ecg2, time = 0.0, time_offset = 0.004;
	string filename = "received/" + (string)"x1.csv";
	ofstream file(filename);
	for (int i = 0; i < dataCount; ++i) {
		reply_ecg1 = request_single_datapoint(channel, patientID, time, 1);
		reply_ecg2 = request_single_datapoint(channel, patientID, time, 2);
		file << time << "," << reply_ecg1 << "," << reply_ecg2 << endl;
		time += time_offset;
	}
}

void post_file_request(FIFORequestChannel* channel, string& filename, __int64_t offset = 0, int length = 0) {
	filemsg msg(offset, length);

	int msgLen = sizeof(filemsg) + (filename.size() + 1);
	char* buffer = new char[msgLen]; 
	memcpy(buffer, &msg, sizeof(filemsg));
	strcpy(buffer + sizeof(filemsg), filename.c_str());
	
	channel->cwrite(buffer, msgLen);

	delete[] buffer;
	buffer = nullptr;
}

void manage_file_request(FIFORequestChannel* channel, string& filename, int buffercapacity) {
	post_file_request(channel, filename);
	__int64_t fileSize;
	channel->cread(&fileSize, sizeof(__int64_t));

	FILE* file_ptr = fopen(("received/" + filename).c_str(), "wb");
	char* responseBuffer = new char[buffercapacity];

	__int64_t position = 0;
	int numReqs = fileSize / buffercapacity;
	for (int i = 0; i < numReqs; ++i) {
		post_file_request(channel, filename, position, buffercapacity);
		channel->cread(responseBuffer, buffercapacity);

		fseek(file_ptr, position, SEEK_SET);
		fwrite(responseBuffer, 1, buffercapacity, file_ptr);

		position += buffercapacity;
	}

	int remaining = fileSize % buffercapacity;
	if (remaining) {
		post_file_request(channel, filename, position, remaining);
		channel->cread(responseBuffer, remaining);

		fseek(file_ptr, position, SEEK_SET);
		fwrite(responseBuffer, 1, remaining, file_ptr);
	}

	if (responseBuffer) {
		delete[] responseBuffer;
		responseBuffer = nullptr;
	}

	fclose(file_ptr);
}

void handle_new_channel(vector<FIFORequestChannel*>& channels) {
	MESSAGE_TYPE newChannelMSG = NEWCHANNEL_MSG;
	channels.front()->cwrite(&newChannelMSG, sizeof(MESSAGE_TYPE));
	char* name = new char[MAX_MESSAGE];
	channels.front()->cread(name, MAX_MESSAGE);
	channels.push_back(new FIFORequestChannel{name, FIFORequestChannel::CLIENT_SIDE});
	delete[] name;
	name = nullptr;
}

int main (int argc, char *argv[]) {
	int opt;
	int patientID;
	double time;
	int ecg;
	int bufferCapacity = MAX_MESSAGE;
	string filename = "";
	bool p = false, t = false, e = false, f = false, c = false;
	char* exec_server_cmd[] = {(char*) "./server", (char*) "", (char*) "", nullptr};
	

	while ((opt = getopt(argc, argv, "p:t:e:f:m:c")) != -1) {
		switch (opt) {
			case 'p':
				p = true;
				patientID = atoi(optarg);
				break;
			case 't':
				t = true;
				time = atof(optarg);
				break;
			case 'e':
				e = true;
				ecg = atoi(optarg);
				break;
			case 'f':
				f = true;
				filename = optarg;
				break;
			case 'm':
				exec_server_cmd[1] = (char*)"-m";
				exec_server_cmd[2] = optarg;
				bufferCapacity = atoi(optarg);
				break;
			case 'c':
				c = true;
				break;
		}
	}

	// TODO: exec the server
	if (fork() == 0) {
		execvp(exec_server_cmd[0], exec_server_cmd);
	}

	vector<FIFORequestChannel*> channels;
    channels.push_back(new FIFORequestChannel{"control", FIFORequestChannel::CLIENT_SIDE});

	if(c) {
		handle_new_channel(channels);
	}
	if (p) {
		if (t && e) {
			double reply = request_single_datapoint(channels.back(), patientID, time, ecg);
			cout << "For person " << patientID << ", at time " << time << ", the value of ecg " << ecg << " is " << reply << endl;
		} else {
			request_n_datapoints(channels.back(), patientID);
		}
	} else if (f) {
		manage_file_request(channels.back(), filename, bufferCapacity);
	}
	for (FIFORequestChannel*& p_channel : channels) {
		MESSAGE_TYPE quit = QUIT_MSG;
    	p_channel->cwrite(&quit, sizeof(MESSAGE_TYPE));
		delete p_channel;
		p_channel = nullptr;
	}

	waitpid(-1, nullptr, 0);

	return 0;
}
