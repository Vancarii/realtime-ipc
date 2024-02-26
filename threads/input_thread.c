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
    char *remoteIP;
    int remotePort;
    int localPort;
} thread_args;


void* inputThread(void* args) {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    thread_args* arguments = (thread_args*)args;
    servaddr.sin_port = htons(arguments->localPort);

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    while (1) {

        char buffer[1024];
        socklen_t len = sizeof(cliaddr);  //len is value/resuslt

        int n = recvfrom(sockfd, (char *)buffer, 1024, 0, ( struct sockaddr *) &cliaddr, &len);

        buffer[n] = '\0'; // Null-terminate the received string

        inputMessage = strdup(buffer); // Duplicate the string to store in the list
        if (inputMessage == NULL) {
            perror("strdup");
            return NULL;
        }

        screen_signal_append_message(inputMessage);

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

void input_thread_init(void* args){
    pthread_create(&input_thread, NULL, inputThread, args);

}


void input_thread_cleanup(){
    // dont free inputMessage here, it is freed in screen_thread

    pthread_cancel(input_thread);
    pthread_join(input_thread, NULL);

}

