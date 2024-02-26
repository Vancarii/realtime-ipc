#include "keyboard_thread.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "output_thread.h"
#include "screen_thread.h"
#include "../shutdown_manager/shutdown_manager.h"


static char* keyboardMessage = NULL;

static pthread_t keyboard_thread;

void* keyboardInputThread(void*) {
    char inputBuffer[1024]; // Buffer to hold input from the keyboard

    while (1) {

        if (fgets(inputBuffer, sizeof(inputBuffer), stdin) != NULL) {

            // Remove newline character at the end of input if present
            inputBuffer[strcspn(inputBuffer, "\n")] = 0;

            // Allocate memory for a new message
            keyboardMessage = strdup(inputBuffer); // Duplicate the string to store in the list
            if (keyboardMessage == NULL) {
                perror("strdup");
                return NULL;
            }

            output_signal_append_message(keyboardMessage);

            if (strcmp(keyboardMessage, "!") == 0){
                printf("keyboard thread should shutdown...\n");
                output_condition_signal();
                screen_condition_signal();
                signal_shutdown();
                return NULL;
            }
        }

    }

    return NULL;
}


void keyboard_thread_init()
{
    pthread_create(&keyboard_thread, NULL, keyboardInputThread, NULL);
}

void keyboard_thread_cleanup()
{
    free(keyboardMessage);
    pthread_cancel(keyboard_thread);
    pthread_join(keyboard_thread, NULL);
}