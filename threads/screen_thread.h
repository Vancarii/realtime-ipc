#ifndef SCREEN_THREAD_H
#define SCREEN_THREAD_H

#include <pthread.h>


void* screenOutputThread(void*);

void screen_signal_append_message(char* keyboardMessage);

void screen_condition_signal();

void screen_thread_init();

void screen_thread_cleanup();

#endif