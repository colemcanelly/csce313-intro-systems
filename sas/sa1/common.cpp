#include "common.h"

void EXITONERROR (std::string msg) {
    perror(msg.c_str());
    exit(-1);
}

int pipe_open(const char* name, int oflag) {
    if(mkfifo(name, 0600) == -1) {
        //if mkfifo failed because the file already exists, ignore the failure silently;
        //otherwise, exit the program
        if(errno != EEXIST) {
            EXITONERROR("pipe_open(mkfifo) failed");
        }
    }
    int fd = open(name, oflag);
    if (fd < 0) {
        std::string msg = "pipe_open(open \"" + std::string(name) + "\") failed";
        EXITONERROR(msg);
    }
    return fd;
}

void pipe_close(const char* name, int fd) {

    if(close(fd) == -1) {
        EXITONERROR("pipe_close(close) failed");
    }
    if(remove(name) == -1) {
        //if remove failed because the file doesn't exist, ignore the failure silently;
        //otherwise, exit the program
        if(errno != ENOENT) {
            EXITONERROR("pipe_close(remove) failed");
        }
    }
}

ssize_t pipe_write(int fd, const void* buf, size_t sz) {
    ssize_t n_bytes = write(fd, buf, sz);
    if (n_bytes < 0) {
        EXITONERROR("pipe_write failed");
    }
    return n_bytes;
}

ssize_t pipe_read(int fd, void* buf, size_t sz) {
    ssize_t n_bytes = read(fd, buf, sz);
    if (n_bytes < 0) {
        EXITONERROR("pipe_read failed");
    }
    return n_bytes;
}