#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "list.c" 
#include "list.h"


#define PORT 12345
#define BUFFER_SIZE 1024

// Assume List and related functions are implemented

// List sendList;
pthread_mutex_t sendListMutex;
pthread_cond_t sendListNotEmptyCond;

// List receiveList;
pthread_mutex_t receiveListMutex;
pthread_cond_t receiveListNotEmptyCond;

typedef struct {
    char *remoteIP;
    int remotePort;
    int localPort;
    List *sendList;
    List *receiveList;
} thread_args;

void freeList(void*) {}


void* keyboardInputThread(void* arg) {
    char inputBuffer[1024]; // Buffer to hold input from the keyboard

    thread_args* arguments = (thread_args*)arg;

    while (fgets(inputBuffer, sizeof(inputBuffer), stdin) != NULL) {
        // Remove newline character at the end of input if present
        inputBuffer[strcspn(inputBuffer, "\n")] = 0;

        // Allocate memory for a new message
        char* message = strdup(inputBuffer); // Duplicate the string to store in the list

        // Lock the mutex before accessing the shared list
        pthread_mutex_lock(&sendListMutex);

        //critical section

        // Add the message to the end of sendList 
        List_append(arguments->sendList, message); 

        // Signal any threads waiting for messages that the list is not empty
        pthread_cond_signal(&sendListNotEmptyCond);

        // Unlock the mutex after modifying the list
        pthread_mutex_unlock(&sendListMutex);

        // check if the message is "!"
        // checks after since we want to send the message before exiting
        if (strcmp(message, "!") == 0) {
            printf("Exiting...\n");
            //free the sendList
            List_free(arguments->sendList, freeList);
            //free the receiveList
            List_free(arguments->receiveList, freeList);
            //exit the program
            exit(0);
        }
    }

    // Optionally handle cleanup and termination signal here

    return NULL;
}


void* udpOutputThread(void* args) {
    int sockfd;
    struct sockaddr_in servaddr;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4

    thread_args* arguments = (thread_args*)args;

    servaddr.sin_addr.s_addr = inet_addr(arguments->remoteIP);
    servaddr.sin_port = htons(arguments->remotePort);

        while (1) {
            pthread_mutex_lock(&sendListMutex);

            // Wait for the list to have messages
            while (arguments->sendList->head == NULL) {
                pthread_cond_wait(&sendListNotEmptyCond, &sendListMutex);
            }


            char* message = List_trim(arguments->sendList); // Implement this function based on your list structure

            pthread_mutex_unlock(&sendListMutex);

            // Send the message
            if (message != NULL) {
                sendto(sockfd, message, strlen(message), 0, (const struct sockaddr*)&servaddr, sizeof(servaddr));
                // free(message); // Don't forget to free the memory
            }
        }

        close(sockfd);
        return NULL;
    }

    void* udpInputThread(void* args) {
        int sockfd;
        struct sockaddr_in servaddr, cliaddr;

        // Creating socket file descriptor
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }

        memset(&servaddr, 0, sizeof(servaddr));
        memset(&cliaddr, 0, sizeof(cliaddr));

        // Filling server information
        servaddr.sin_family = AF_INET; // IPv4
        servaddr.sin_addr.s_addr = INADDR_ANY;
        thread_args* arguments = (thread_args*)args;
        servaddr.sin_port = htons(arguments->localPort);

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        char buffer[1024];
        socklen_t len = sizeof(cliaddr);  //len is value/resuslt

        int n = recvfrom(sockfd, (char *)buffer, 1024, 0, ( struct sockaddr *) &cliaddr, &len);
        buffer[n] = '\0'; // Null-terminate the received string

        // Lock the mutex before accessing the shared list
        pthread_mutex_lock(&receiveListMutex);

        // Add the received message to the end of receiveList
        List_append(arguments->receiveList, buffer); 

        // Signal any threads waiting for messages that the list is not empty
        pthread_cond_signal(&receiveListNotEmptyCond);

        // Unlock the mutex after modifying the list
        pthread_mutex_unlock(&receiveListMutex);
    }

    close(sockfd);
    return NULL;
}

void* screenOutputThread(void* args) {

    thread_args* arguments = (thread_args*)args;

    while (1) {
        // Lock the mutex before accessing the shared list
        pthread_mutex_lock(&receiveListMutex);

        // Wait for the list to have messages
        while (arguments->receiveList->head == NULL) {
            pthread_cond_wait(&receiveListNotEmptyCond, &receiveListMutex);
        }

        // Assuming list_remove returns a dynamically allocated string
        // that needs to be freed after use
        char* message = List_trim(arguments->receiveList);

        // Unlock the mutex after removing the message from the list
        pthread_mutex_unlock(&receiveListMutex);

        // Display the message
        if (message != NULL) {
            printf("%s\n", message);
            // free(message); // Don't forget to free the memory
        }

        if (strcmp(message, "!") == 0) {
            printf("Exiting...\n");
            //free the sendList
            List_free(arguments->sendList, freeList);
            //free the receiveList
            List_free(arguments->receiveList, freeList);
            //exit the program
            exit(0);
        }
    }

    return NULL;
}


int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s local_port remote_ip remote_port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    printf("Starting s-talk...\n");
    printf("Local port: %s\n", argv[1]);
    printf("Remote IP: %s\n", argv[2]);
    printf("Remote port: %s\n", argv[3]);

    pthread_mutex_init(&sendListMutex, NULL);
    pthread_mutex_init(&receiveListMutex, NULL);
    pthread_cond_init(&sendListNotEmptyCond, NULL);
    pthread_cond_init(&receiveListNotEmptyCond, NULL);

    thread_args *args = malloc(sizeof *args);
    args->localPort = atoi(argv[1]);
    args->remoteIP = argv[2];
    args->remotePort = atoi(argv[3]);

    pthread_t threads[4];


    // Initialize sendList and receiveList
    args->sendList = List_create();
    args->receiveList = List_create();


    // Create threads
    pthread_create(&threads[0], NULL, keyboardInputThread, args);
    pthread_create(&threads[1], NULL, udpOutputThread, args);
    pthread_create(&threads[2], NULL, udpInputThread, args);
    pthread_create(&threads[3], NULL, screenOutputThread, args);

    // Wait for threads to finish
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    // Cleanup
    pthread_mutex_destroy(&sendListMutex);
    pthread_mutex_destroy(&receiveListMutex);
    pthread_cond_destroy(&sendListNotEmptyCond);
    pthread_cond_destroy(&receiveListNotEmptyCond);

    List_free(args->sendList, freeList);
    List_free(args->receiveList, freeList);
    return 0;
}