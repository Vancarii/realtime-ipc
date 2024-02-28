#include "screen_thread.h"
#include <stdio.h>
#include <stdlib.h>
#include "../list/list.h"
#include "../shutdown_manager/shutdown_manager.h"

static pthread_t screen_thread;

// list to be shared by this thread and the UDP input Thread
static List *receiveList;

static pthread_mutex_t receiveListMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t receiveListNotEmptyCond = PTHREAD_COND_INITIALIZER;


void* screenOutputThread(void* unused) {

    while (1) {

        pthread_mutex_lock(&receiveListMutex);
    
        // Wait for the list to have messages
        while (receiveList->head == NULL) {

            if (should_shutdown()){
                return NULL;
            }

            pthread_cond_wait(&receiveListNotEmptyCond, &receiveListMutex);
        }

        // Remove the most recently appended message from the list
        char* message = List_trim(receiveList);

        pthread_mutex_unlock(&receiveListMutex);

        // Display the message
        // free the message and check for shutdown signal
        if (message != NULL) {
            fputs(message, stdout);
            fputs("\n", stdout);

            free(message);

            if (should_shutdown()){
                return NULL;
            }

        }

    }

    return NULL;
}


// this function is called by the UDP input Thread to signal that
// there is something to put onto the list
// and this function appends the message and changes the condition
void screen_signal_append_message(char* inputMessage) {
    pthread_mutex_lock(&receiveListMutex);
    {
        // Add the received message to the end of receiveList
        List_append(receiveList, inputMessage); 

        // Signal any threads waiting for messages that the list is not empty
        pthread_cond_signal(&receiveListNotEmptyCond);
    }

    pthread_mutex_unlock(&receiveListMutex);
}

// this function is called by the output threads signal that
// it should continue its process since we are shutting down and so that
// this thread can check the shutdown condition
void screen_condition_signal() {
    pthread_mutex_lock(&receiveListMutex);

    pthread_cond_signal(&receiveListNotEmptyCond);

    pthread_mutex_unlock(&receiveListMutex);
}


// initialization
void screen_thread_init(){

    receiveList = List_create();
    
    pthread_create(&screen_thread, NULL, screenOutputThread, NULL);

}

// cleanup
void screen_thread_cleanup(){


    pthread_mutex_destroy(&receiveListMutex);
    pthread_cond_destroy(&receiveListNotEmptyCond);

    pthread_join(screen_thread, NULL);
    
    List_free(receiveList, free);

}


