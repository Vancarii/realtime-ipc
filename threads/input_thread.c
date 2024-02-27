#include "input_thread.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "screen_thread.h"
#include "output_thread.h"
#include "../list/list.h"
#include "../shutdown_manager/shutdown_manager.h"


static pthread_t input_thread;

static char* inputMessage = NULL;

typedef struct {
    char *remoteHostname;
    int remotePort;
    int localPort;
} thread_args;


// All thread operations 
// This thread is a socket that receives from the UDP output thread
// once messages are received, they are stored in the list
// if the message is "!", the shutdown signal is sent
// and the thread is terminated
void* inputThread(void* args) {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    thread_args* arguments = (thread_args*)args;
    servaddr.sin_port = htons(arguments->localPort);

    // Binding the socket 
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    while (1) {

        char buffer[1024];
        socklen_t len = sizeof(cliaddr);

        int n = recvfrom(sockfd, (char *)buffer, 1024, 0, ( struct sockaddr *) &cliaddr, &len);

        buffer[n] = '\0';

        // copies the message to a new string
        inputMessage = strdup(buffer);
        // null check
        if (inputMessage == NULL) {
            perror("strdup");
            return NULL;
        }

        // calls the screen thread to append the message to the list
        // function is written in the screen module so that the list and 
        // mutexes are encapsulated inside of screen module and not easily changed
        // here in this thread by accident
        screen_signal_append_message(inputMessage);

        // end case termination
        // this also signals the output and screen threads to be unblocked 
        // and continue their process so that they can check the shutdown flag
        // then signal the shutdown
        if (strcmp(inputMessage, "!") == 0){
            output_condition_signal();
            screen_condition_signal();
            signal_shutdown();
            return NULL;
        }

    }

    close(sockfd);

    return NULL;
}

// initialization 
void input_thread_init(void* args){
    pthread_create(&input_thread, NULL, inputThread, args);
}


void input_thread_cleanup(){
    // dont free inputMessage here, it is freed in screen_thread

    // cancel the thread since there is a system call
    // pthread_cancel is used to cancel the system call and cancel the thread
    pthread_cancel(input_thread);
    pthread_join(input_thread, NULL);
}

