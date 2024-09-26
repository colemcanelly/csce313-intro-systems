#include "common.h"

using namespace std;

void run_pipe_logic(int rfd, int wfd, string name, int number){
    printf("Client request: \"%s\", %d\n", name.c_str(), number);

    //create the request object
    Request req;
    //create the reply object
    char reply[MAX_MESSAGE];

    //=====================================================================
    //BEGIN SKILL ASSESSMENT FILL-IN PORTION 1
    //=====================================================================

    memset(&req, 0, sizeof(req));       //DONE: initialize req to zero
    strcpy(req.name, name.c_str());     //DONE: copy the name parameter into req.name
    req.number = number;                //DONE: copy the number parameter into req.number

    //DONE: write the request to the server
    ssize_t written = pipe_write(wfd, &req, sizeof(req));
    
    //=====================================================================
    //END SKILL ASSESSMENT FILL-IN PORTION 1
    //=====================================================================
    
    printf("Client sent %ld/%lu bytes\n", written, sizeof(req));
    //This sleep here ensures the server has time
    //to read the client's request and send a response;
    //without it, the server may experience a broken pipe
    //if the client doesn't wait for the server's reply.
    //Basically, it makes it so that you can complete the first part
    //and then test it out in the autograder
    //without having to finish the second part as well.
    sleep(1);
    
    //=====================================================================
    //BEGIN SKILL ASSESSMENT FILL-IN PORTION 2
    //=====================================================================
    
    memset(reply, 0, MAX_MESSAGE);                          //DONE: initialize reply to zero
    ssize_t recvlen = pipe_read(rfd, reply, MAX_MESSAGE);   //DONE: read the server's reply

    //=====================================================================
    //END SKILL ASSESSMENT FILL-IN PORTION 2
    //=====================================================================

    printf("Client received %ld bytes: \"%s\"\n", recvlen, reply);
}

int main(int argc, char* argv[]){
    //parse cmdline arguments
    string name = "";
    int number = -1;
    int opt;
    while((opt = getopt(argc, argv, "n:i:")) != -1){
        switch(opt) {
            case 'n':
                name = optarg;
                break;
            case 'i':
                number = atoi(optarg);
                break;
        }
    }
    //verify cmdline arguments
    if (name == "" || number == -1){
        printf("Usage: %s -n <name> -i <number>\n", argv[0]);
        return 0;
    }

    string pipeWrite = "pipe_client_to_server";
    string pipeRead = "pipe_server_to_client";
    
    //=====================================================================
    //BEGIN SKILL ASSESSMENT FILL-IN PORTION 3
    //=====================================================================
    
    //TODO: open the client's read and write pipes
    int wfd = pipe_open(pipeWrite.c_str(), O_WRONLY);
    int rfd = pipe_open(pipeRead.c_str(), O_RDONLY);

    //=====================================================================
    //END SKILL ASSESSMENT FILL-IN PORTION 3
    //=====================================================================

    run_pipe_logic(rfd, wfd, name, number);

    pipe_close(pipeRead.c_str(), rfd);
    pipe_close(pipeWrite.c_str(), wfd);

    return 0;
}