#ifndef SHUTDOWN_MANAGER_H
#define SHUTDOWN_MANAGER_H

#include <pthread.h>

extern pthread_cond_t shutdownCond;
extern pthread_mutex_t shutdownMutex;

void init_shutdown_manager();

void waitFor_shutdown();

void signal_shutdown();

int should_shutdown();

void cleanup_shutdown_manager();

#endif
