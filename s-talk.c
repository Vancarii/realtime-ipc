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


typedef struct {
    char *remoteIP;
    int remotePort;
    int localPort;
} thread_args;



int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s local_port remote_ip remote_port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    init_shutdown_manager();

    thread_args *args = malloc(sizeof *args);
    args->localPort = atoi(argv[1]);
    args->remoteIP = argv[2];
    args->remotePort = atoi(argv[3]);


    // initialize threads

    keyboard_thread_init();
    output_thread_init(args);

    screen_thread_init();
    input_thread_init(args);

    // shutdown manager
    waitFor_shutdown();

    //cleanup

    keyboard_thread_cleanup();
    output_thread_cleanup();

    screen_thread_cleanup();
    input_thread_cleanup();
    
    cleanup_shutdown_manager();

    free(args);

    return 0;
}