#include "common.h"

void EXITONERROR (std::string msg) {
    perror(msg.c_str());
    exit(-1);
}

int open_pipe (std::string name, int mode) {
    // make fifo
    int fd = 0;// open pipe;
    if (fd < 0) {
        EXITONERROR(name);
    }
    return fd;
}

void close_pipe (std::string name, int fd) {
    
}

int pipe_read (int rfd, void* msgbuf, int msgsize) {
    int n_bytes = 0;// read from pipe;
    if (n_bytes < 0) {
        EXITONERROR("read");
    }
    return n_bytes;
}

int pipe_write (int wfd, void* msgbuf, int msgsize) {
    int n_bytes = 0;// write to pipe;
    if (n_bytes < 0) {
        EXITONERROR("write");
    }
    return n_bytes;
}
