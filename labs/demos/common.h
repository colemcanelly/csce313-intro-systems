#ifndef _COMMON_H_
#define _COMMON_H_

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_MESSAGE 256

typedef char byte_t;

enum REQUEST_TYPE {UNKNOWN_REQ, TYPE_REQ, WORD_REQ, QUIT};

class TypeReq {
public:
    REQUEST_TYPE reqtype;
    int wordtype;

    TypeReq (int _wordtype) : reqtype(TYPE_REQ), wordtype(_wordtype) {}
};

class WordReq {
public:
    REQUEST_TYPE reqtype;
    int wordtype;
    int wordind;

    WordReq (int _wordtype, int _wordind) : reqtype(WORD_REQ), wordtype(_wordtype), wordind(_wordind) {}
};

void EXITONERROR (std::string msg);

int open_pipe (std::string name, int mode);

void close_pipe (std::string name, int fd);

int pipe_read (int rfd, void* msgbuf, int msgsize);

int pipe_write (int wfd, void* msgbuf, int msgsize);

#endif
