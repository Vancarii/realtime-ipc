#ifndef OUTPUT_THREAD_H
#define OUTPUT_THREAD_H

#include <pthread.h>


void freeList(void* item);

void* udpOutputThread(void* args);

void output_signal_append_message(char* keyboardMessage);

void output_condition_signal();

void output_thread_init(void *args);
void output_thread_cleanup();

#endif