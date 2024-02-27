#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> 
#include "output_thread.h"
#include "../list/list.h"
#include "../shutdown_manager/shutdown_manager.h"

static pthread_t output_thread;

// list to be shared by UDP output and keyboard thread
static List *sendList;

static pthread_mutex_t sendListMutex;
static pthread_cond_t sendListNotEmptyCond;

typedef struct {
    char *remoteHostname;
    int remotePort;
    int localPort;
} thread_args;


void* udpOutputThread(void* args) {
    int sockfd;

    struct addrinfo hints, *res;
    int status;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    thread_args* arguments = (thread_args*)args;

    char portStr[6];
    sprintf(portStr, "%d", arguments->remotePort);

    if ((status = getaddrinfo(arguments->remoteHostname, portStr, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    while (1) {

        pthread_mutex_lock(&sendListMutex);
        
        // Wait for the list to have messages
        while (sendList->head == NULL) {

            if (should_shutdown()){
                freeaddrinfo(res);
                close(sockfd);
                return NULL;
            }

            pthread_cond_wait(&sendListNotEmptyCond, &sendListMutex);
        }

        char* message = List_trim(sendList);
    
        pthread_mutex_unlock(&sendListMutex);


        // Send the message
        // make sure you free the message after sending it
        // this message also points to the same memory location as the keyboardMessage
        // pointer in the keyboard thread file
        // so we free it here and not in keyboard file
        if (message != NULL) {
            sendto(sockfd, message, strlen(message), 0, res->ai_addr, res->ai_addrlen);

            free(message);

            // check for shutdown signal
            // make sure to free allocated and close the socket if returning
            if (should_shutdown()){
                freeaddrinfo(res);
                close(sockfd);
                return NULL;
            }

        }

    }

    freeaddrinfo(res);
    close(sockfd);
    return NULL;
}



// This function is called by the keyboard Thread to signal that 
// there is something to put onto the list
// and this function appends the message and changes the condition
void output_signal_append_message(char* keyboardMessage){

    pthread_mutex_lock(&sendListMutex);
    
    // Add the message to the end of sendList 
    List_append(sendList, keyboardMessage); 

    // Signal any threads waiting for messages that the list is not empty
    pthread_cond_signal(&sendListNotEmptyCond);


    // Unlock the mutex after modifying the list
    pthread_mutex_unlock(&sendListMutex);

}

// This function is called by the input Thread to signal that
// this thread should not be blocked anymore when the shutdown signal is sent
void output_condition_signal(){
    pthread_cond_signal(&sendListNotEmptyCond);
}


// initialization
void output_thread_init(void *args)
{
    sendList = List_create();

    pthread_mutex_init(&sendListMutex, NULL);
    pthread_cond_init(&sendListNotEmptyCond, NULL);

    pthread_create(&output_thread, NULL, udpOutputThread, args);
}


// cleanup
// make sure to cancel and join the thread before destroying the mutexes and conditions
// this way the thread is not left running and the mutexes and conditions are not destroyed
void output_thread_cleanup()
{
    pthread_cond_broadcast(&sendListNotEmptyCond);

    pthread_cancel(output_thread);
    pthread_join(output_thread, NULL);

    pthread_mutex_destroy(&sendListMutex);
    pthread_cond_destroy(&sendListNotEmptyCond);

    List_free(sendList, free);

}