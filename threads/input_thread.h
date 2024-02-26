#ifndef INPUT_THREAD_H
#define INPUT_THREAD_H

#include <pthread.h>

void* inputThread(void* args);

void input_thread_init(void* args);

void input_thread_cleanup();

#endif
