#include "shutdown_manager.h"
#include <pthread.h>
#include <stdio.h>

int shutdown_flag;
pthread_cond_t shutdownCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t shutdownMutex = PTHREAD_MUTEX_INITIALIZER;

void init_shutdown_manager() {
    shutdown_flag = 0;
    pthread_mutex_init(&shutdownMutex, NULL);
    pthread_cond_init(&shutdownCond, NULL);
}

void waitFor_shutdown() {
    pthread_mutex_lock(&shutdownMutex);
    while (!shutdown_flag) {
        printf("Waiting for broadcast...\n"); 
        pthread_cond_wait(&shutdownCond, &shutdownMutex);
        printf("received broadcast \n"); 
    }
    pthread_mutex_unlock(&shutdownMutex);
}

void signal_shutdown() {
    pthread_mutex_lock(&shutdownMutex);
    shutdown_flag = 1;
    // printf("broadcast cond var...\n");
    int ret = pthread_cond_broadcast(&shutdownCond);
    if (ret == 0) {
        printf("Broadcast sent successfully.\n");
    } else {
        printf("Failed to send broadcast. Error: %d\n", ret);
    }
    pthread_mutex_unlock(&shutdownMutex);
}


int should_shutdown() {
    int flag;
    pthread_mutex_lock(&shutdownMutex);
    flag = shutdown_flag;
    pthread_mutex_unlock(&shutdownMutex);
    return flag;
}

void cleanup_shutdown_manager() {
    pthread_mutex_destroy(&shutdownMutex);
    pthread_cond_destroy(&shutdownCond);
}
