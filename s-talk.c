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
#include "socket.h"
#include "args_struct.h"


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
    

    // socket cleanup
    freeaddrinfo(args->res);
    close(args->socket);
    free(args);

    // properly cleanup shutdown
    cleanup_shutdown_manager();

    return 0;
}