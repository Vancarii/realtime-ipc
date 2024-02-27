#include "keyboard_thread.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "output_thread.h"
#include "screen_thread.h"
#include "../shutdown_manager/shutdown_manager.h"


static char* keyboardMessage = NULL;

static pthread_t keyboard_thread;

void* keyboardInputThread(void* unused) {

    char inputBuffer[1024];

    while (1) {

        if (fgets(inputBuffer, sizeof(inputBuffer), stdin) != NULL) {

            // Remove newline character at the end of input
            inputBuffer[strcspn(inputBuffer, "\n")] = 0;

            // Allocate memory for a new message
            // duplicates the string into keyboardMessage
            keyboardMessage = strdup(inputBuffer);
            // null check
            if (keyboardMessage == NULL) {
                perror("strdup");
                return NULL;
            }

            // tells the output thread to append the message to the list
            // this way the mutexes and the list are encapsulated inside of the output module
            // and not easily changed here in this thread by accident
            output_signal_append_message(keyboardMessage);

            // end case termination
            // this also signals the output and screen threads to be unblocked
            // and continue their process so that they can check the shutdown flag
            if (strcmp(keyboardMessage, "!") == 0){
                output_condition_signal();
                screen_condition_signal();
                signal_shutdown();
                return NULL;
            }
        }

    }

    return NULL;
}


// initialization
void keyboard_thread_init()
{
    pthread_create(&keyboard_thread, NULL, keyboardInputThread, NULL);
}


// cleanup
void keyboard_thread_cleanup()
{
    // dont free keyboardMessage here, it is freed in output_thread
    pthread_cancel(keyboard_thread);
    pthread_join(keyboard_thread, NULL);
}