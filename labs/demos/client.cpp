#include "common.h"
#include <iostream>
#include <string.h>

using namespace std;

int main (int argc, char** argv) {
    mkfifo("client_read", 0666);
    int read_fd = open("client_read", O_RDONLY);
    mkfifo("server_read", 0666);
    int write_fd = open("server_read", O_WRONLY);
    
    string to_write = "howdy world!";
    write(write_fd, to_write.c_str(), to_write.length() + 1); 

    char received[MAX_MESSAGE];
    memset(received, 0, MAX_MESSAGE);
    int nbytes = read(read_fd, received, MAX_MESSAGE);

    cout << "Received: " << received << endl;
}
