#include "common.h"

#include <fstream>
#include <iostream>

using namespace std;

string find_in_db(string name, int number){
    fstream ifs("test-files/names.csv");
    string line;
    while(getline(ifs, line)){
        size_t pos = line.find(',');
        string line_name = line.substr(0, pos);

        if (line_name == name){
            int line_number = std::stoi(line.substr(pos + 1));
            if(line_number == number) {
                return "SUCCESS";
            } else {
                return "INCORRECT NUMBER";
            }
        }
    }
    return "NAME NOT FOUND";
}

void run_pipe_logic(int rfd, int wfd){
    //declare the request and reply objects
    Request req;
    char reply[MAX_MESSAGE];

    //initialize both to zero
    memset(&req, 0, sizeof(req));
    memset(reply, 0, MAX_MESSAGE); //it's not good to use sizeof on an array :^)

    //read the client's request
    ssize_t recvlen = pipe_read(rfd, &req, sizeof(req));

    printf("Server received %ld bytes: \"%s\", %d\n", recvlen, req.name, req.number);

    //process the request
    string result = "NO NAME RECEIVED";
    if (req.name[0] != 0){
        result = find_in_db(req.name, req.number);
    }

    printf("Server reply: \"%s\"\n", result.c_str());

    //send a reply
    strcpy(reply, result.c_str());
    size_t sendlen = strlen(reply) + 1;
    ssize_t written = pipe_write(wfd, reply, sendlen);

    printf("Server sent %ld/%lu bytes\n", written, sendlen);

}

int main(){

    string pipeRead = "pipe_client_to_server";
    string pipeWrite = "pipe_server_to_client";
    int rfd = pipe_open(pipeRead.c_str(), O_RDONLY);
    int wfd = pipe_open(pipeWrite.c_str(), O_WRONLY);

    run_pipe_logic(rfd, wfd);

    pipe_close(pipeRead.c_str(), rfd);
    pipe_close(pipeWrite.c_str(), wfd);

    return 0;
}