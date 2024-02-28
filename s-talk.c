#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "list/list.h"
#include "shutdown_manager/shutdown_manager.h"
#include "threads/keyboard_thread.h"
#include "threads/output_thread.h"
#include "threads/screen_thread.h"
#include "threads/input_thread.h"
#include <netdb.h>



// struct to store user input arguments
// to be passed to the threads in one argument
typedef struct {
    char *remoteHostname;
    int remotePort;
    int localPort;
    int socket;
    struct addrinfo *res;
} thread_args;


void socket_creation(thread_args* args){

    int sockfd;

    struct addrinfo hints;
    struct addrinfo *res;

    struct sockaddr_in servaddr;


    int status;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;

    thread_args* arguments = (thread_args*)args;

    servaddr.sin_port = htons(arguments->localPort);

    // Binding the socket 
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    char portStr[6];
    sprintf(portStr, "%d", arguments->remotePort);

    if ((status = getaddrinfo(arguments->remoteHostname, portStr, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }


    arguments->socket = sockfd;
    arguments->res = res;

}


int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s local_port remote_ip remote_port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // initialize shutdown manager
    init_shutdown_manager();

    // store user input arguments into struct to be passed to threads
    thread_args *args = malloc(sizeof *args);
    args->localPort = atoi(argv[1]);
    args->remoteHostname = argv[2];
    args->remotePort = atoi(argv[3]);


    socket_creation(args);

    // initialize threads

    keyboard_thread_init();
    output_thread_init(args);

    screen_thread_init();
    input_thread_init(args);

    // shutdown manager
    // This is a blocking call that will wait for the shutdown signal
    // main thread gets blocked until the signal unblocks it
    // signal is sent from threads that receive "!" termination input
    waitFor_shutdown();

    //cleanup

    keyboard_thread_cleanup();
    output_thread_cleanup();

    screen_thread_cleanup();
    input_thread_cleanup();
    


    // free memory
    freeaddrinfo(args->res);
    close(args->socket);
    free(args);

    // properly cleanup shutdown
    cleanup_shutdown_manager();

    return 0;
}