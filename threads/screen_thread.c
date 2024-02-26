#include "screen_thread.h"
#include <stdio.h>
#include <stdlib.h>
#include "../list/list.h"
#include "../shutdown_manager/shutdown_manager.h"

static pthread_t screen_thread;

static List *receiveList;

static pthread_mutex_t receiveListMutex;
static pthread_cond_t receiveListNotEmptyCond;


void* screenOutputThread(void* unused) {

    while (1) {

        // Lock the mutex before accessing the shared list
        pthread_mutex_lock(&receiveListMutex);
    
        // Wait for the list to have messages
        while (receiveList->head == NULL) {

            if (should_shutdown()){
                return NULL;
            }

            pthread_cond_wait(&receiveListNotEmptyCond, &receiveListMutex);
        }

        // Assuming list_remove returns a dynamically allocated string
        // that needs to be freed after use
        char* message = List_trim(receiveList);

        // Unlock the mutex after removing the message from the list
        pthread_mutex_unlock(&receiveListMutex);

        // Display the message
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



void screen_signal_append_message(char* inputMessage) {
            // Lock the mutex before accessing the shared list
    pthread_mutex_lock(&receiveListMutex);
    {
        // Add the received message to the end of receiveList
        List_append(receiveList, inputMessage); 

        // Signal any threads waiting for messages that the list is not empty
        pthread_cond_signal(&receiveListNotEmptyCond);
    }
    // Unlock the mutex after modifying the list
    pthread_mutex_unlock(&receiveListMutex);
}

void screen_condition_signal() {
    pthread_cond_signal(&receiveListNotEmptyCond);
}



void screen_thread_init(){

    receiveList = List_create();

    pthread_mutex_init(&receiveListMutex, NULL);
    pthread_cond_init(&receiveListNotEmptyCond, NULL);
    
    pthread_create(&screen_thread, NULL, screenOutputThread, NULL);

}
void screen_thread_cleanup(){

    List_free(receiveList, free);

    pthread_mutex_destroy(&receiveListMutex);
    pthread_cond_destroy(&receiveListNotEmptyCond);

    pthread_join(screen_thread, NULL);

}


