#include "shutdown_manager.h"
#include <pthread.h>
#include <stdio.h>


// global variables

int shutdown_flag;
pthread_cond_t shutdownCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t shutdownMutex = PTHREAD_MUTEX_INITIALIZER;


// initialize all variables, mutexes and conditions
void init_shutdown_manager() {
    shutdown_flag = 0;
    pthread_mutex_init(&shutdownMutex, NULL);
    pthread_cond_init(&shutdownCond, NULL);
}

// Blocking operation to block the main thread
// waits for the signal to change the shutdown_flag
void waitFor_shutdown() {
    pthread_mutex_lock(&shutdownMutex);
    while (!shutdown_flag) {
        pthread_cond_wait(&shutdownCond, &shutdownMutex);
    }
    pthread_mutex_unlock(&shutdownMutex);
}

// called by any thread that receives the "!" termination input
// changes the flag to true so that the main thread can unblock the main thread
// and start the cleanup process
void signal_shutdown() {
    pthread_mutex_lock(&shutdownMutex);
    shutdown_flag = 1;
    pthread_cond_broadcast(&shutdownCond);
    pthread_mutex_unlock(&shutdownMutex);
}


// returns the value of the shutdown_flag
// used by the threads to check if the shutdown signal has been sent
int should_shutdown() {
    int flag;
    pthread_mutex_lock(&shutdownMutex);
    flag = shutdown_flag;
    pthread_mutex_unlock(&shutdownMutex);
    return flag;
}

// cleaup 
void cleanup_shutdown_manager() {
    pthread_mutex_destroy(&shutdownMutex);
    pthread_cond_destroy(&shutdownCond);
}
