#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "output_thread.h"
#include "../list/list.h"
#include "../shutdown_manager/shutdown_manager.h"

static pthread_t output_thread;

static List *sendList;

static pthread_mutex_t sendListMutex;
static pthread_cond_t sendListNotEmptyCond;


typedef struct {
    char *remoteIP;
    int remotePort;
    int localPort;
} thread_args;


void* udpOutputThread(void* args) {
    int sockfd;
    struct sockaddr_in servaddr;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4

    thread_args* arguments = (thread_args*)args;

    servaddr.sin_addr.s_addr = inet_addr(arguments->remoteIP);
    servaddr.sin_port = htons(arguments->remotePort);

    while (1) {

        pthread_mutex_lock(&sendListMutex);
        
        // Wait for the list to have messages
        while (sendList->head == NULL) {

            if (should_shutdown()){
                return NULL;
            }

            pthread_cond_wait(&sendListNotEmptyCond, &sendListMutex);
        }

        char* message = List_trim(sendList);
    
        pthread_mutex_unlock(&sendListMutex);


        // Send the message
        if (message != NULL) {
            sendto(sockfd, message, strlen(message), 0, (const struct sockaddr*)&servaddr, sizeof(servaddr));

            free(message);

            if (should_shutdown()){
                return NULL;
            }

        }

    }

    close(sockfd);
    return NULL;
}


void output_signal_append_message(char* keyboardMessage){
        // Lock the mutex before accessing the shared list
    pthread_mutex_lock(&sendListMutex);
    
    // Add the message to the end of sendList 
    List_append(sendList, keyboardMessage); 

    // Signal any threads waiting for messages that the list is not empty
    pthread_cond_signal(&sendListNotEmptyCond);


    // Unlock the mutex after modifying the list
    pthread_mutex_unlock(&sendListMutex);

}

void output_condition_signal(){
    pthread_cond_signal(&sendListNotEmptyCond);
}



void output_thread_init(void *args)
{

    sendList = List_create();

    pthread_mutex_init(&sendListMutex, NULL);
    pthread_cond_init(&sendListNotEmptyCond, NULL);

    pthread_create(&output_thread, NULL, udpOutputThread, args);
}



void output_thread_cleanup()
{
    pthread_mutex_destroy(&sendListMutex);
    pthread_cond_destroy(&sendListNotEmptyCond);

    List_free(sendList, free);

    pthread_cancel(output_thread);
    pthread_join(output_thread, NULL);
}