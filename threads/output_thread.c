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

static pthread_mutex_t sendListMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t sendListNotEmptyCond = PTHREAD_COND_INITIALIZER;

typedef struct {
    char *remoteHostname;
    int remotePort;
    int localPort;
    int socket;
    struct addrinfo *res;
} thread_args;


void* udpOutputThread(void* args) {

    thread_args* arguments = (thread_args*)args;

    int sockfd = arguments->socket;
    struct addrinfo *res = arguments->res;

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
        // make sure you free the message after sending it
        // this message also points to the same memory location as the keyboardMessage
        // pointer in the keyboard thread file
        // so we free it here and not in keyboard file
        if (message != NULL) {


            sendto(sockfd, message, strlen(message), 0, res->ai_addr, res->ai_addrlen);

            free(message);

            // check for shutdown signal
            if (should_shutdown()){
                return NULL;
            }

        }

    }

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

    pthread_mutex_lock(&sendListMutex);

    pthread_cond_signal(&sendListNotEmptyCond);

    pthread_mutex_unlock(&sendListMutex);
}


// initialization
void output_thread_init(void *args)
{
    sendList = List_create();

    pthread_create(&output_thread, NULL, udpOutputThread, args);
}


// cleanup
// make sure to cancel and join the thread before destroying the mutexes and conditions
// this way the thread is not left running and the mutexes and conditions are not destroyed
void output_thread_cleanup()
{


    pthread_mutex_destroy(&sendListMutex);
    pthread_cond_destroy(&sendListNotEmptyCond);


    pthread_cancel(output_thread);
    pthread_join(output_thread, NULL);

    List_free(sendList, free);


}