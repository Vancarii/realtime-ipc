#ifndef KEYBOARD_THREAD_H
#define KEYBOARD_THREAD_H

#include <pthread.h>

void* keyboardInputThread(void*);

void keyboard_thread_init();
void keyboard_thread_cleanup();

#endif