#ifndef SHUTDOWN_MANAGER_H
#define SHUTDOWN_MANAGER_H

#include <pthread.h>

extern pthread_cond_t shutdownCond;
extern pthread_mutex_t shutdownMutex;

// Initializes the shutdown manager
void init_shutdown_manager();

// waits for the shutdown when flag is false / when not needed to shutdown yet
void waitFor_shutdown();

// Signals the shutdown process to start
void signal_shutdown();

// Checks if a shutdown signal has been received
int should_shutdown();

// Cleans up resources used by the shutdown manager
void cleanup_shutdown_manager();

#endif
