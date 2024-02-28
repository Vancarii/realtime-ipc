#ifndef ARGS_STRUCT_H
#define ARGS_STRUCT_H


// struct to store user input arguments
// to be passed to the threads in one argument
typedef struct {
    char *remoteHostname;
    int remotePort;
    int localPort;
    int socket;
    struct addrinfo *res;
} thread_args;

#endif