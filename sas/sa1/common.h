#pragma once //this is a special type of include guard, pls ignore :)

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <string>

#define MAX_MESSAGE 256

struct Request {
    char name[MAX_MESSAGE];
    int number;
};

void EXITONERROR (std::string msg);

//helpful pipe functions
int pipe_open(const char* name, int oflag);
void pipe_close(const char* name, int fd);
ssize_t pipe_write(int fd, const void* buf, size_t sz);
ssize_t pipe_read(int fd, void* buf, size_t sz);