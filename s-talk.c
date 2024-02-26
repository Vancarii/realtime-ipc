#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "list.h"
#include "shutdown_manager.h"


// lists
static List *sendList;
static List *receiveList;

// List sendList;
static pthread_mutex_t sendListMutex;
static pthread_cond_t sendListNotEmptyCond;

// List receiveList;
static pthread_mutex_t receiveListMutex;
static pthread_cond_t receiveListNotEmptyCond;

typedef struct {
    char *remoteIP;
    int remotePort;
    int localPort;
    // List *sendList;
    // List *receiveList;
} thread_args;

void freeList(void* item) {
    free(item);
}

static char* keyboardMessage = NULL;
static char* inputMessage = NULL;


void* keyboardInputThread(void*) {
    char inputBuffer[1024]; // Buffer to hold input from the keyboard

    // thread_args* arguments = (thread_args*)args;

    if (sendList == NULL) {
        printf("sendList is NULL\n");
        return NULL;
    }

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

            // Lock the mutex before accessing the shared list
            pthread_mutex_lock(&sendListMutex);
            
            // Add the message to the end of sendList 
            List_append(sendList, keyboardMessage); 

            // Signal any threads waiting for messages that the list is not empty
            pthread_cond_signal(&sendListNotEmptyCond);

        
            // Unlock the mutex after modifying the list
            pthread_mutex_unlock(&sendListMutex);

            // waitFor_shutdown();

            if (strcmp(keyboardMessage, "!") == 0){
                printf("keyboard thread should shutdown...\n");
                pthread_cond_signal(&sendListNotEmptyCond);
                pthread_cond_signal(&receiveListNotEmptyCond);
                signal_shutdown();
                return NULL;
            }
        }

    }

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
        while (sendList->head == NULL) {

            if (should_shutdown()){
                printf("1. output thread should shutdown...\n");
                return NULL;
            }

            pthread_cond_wait(&sendListNotEmptyCond, &sendListMutex);
        }

        char* message = List_trim(sendList);
    
        pthread_mutex_unlock(&sendListMutex);


        // Send the message
        if (message != NULL) {
            sendto(sockfd, message, strlen(message), 0, (const struct sockaddr*)&servaddr, sizeof(servaddr));
            // free(message); // Don't forget to free the memory

            if (should_shutdown()){
                printf("2. output thread should shutdown...\n");
                free(message);
                return NULL;
            }


            free(message);
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

        inputMessage = strdup(buffer); // Duplicate the string to store in the list
        if (inputMessage == NULL) {
            perror("strdup");
            return NULL;
        }

        // Lock the mutex before accessing the shared list
        pthread_mutex_lock(&receiveListMutex);
        {
            // Add the received message to the end of receiveList
            List_append(receiveList, inputMessage); 

            // Signal any threads waiting for messages that the list is not empty
            pthread_cond_signal(&receiveListNotEmptyCond);
        }
        // Unlock the mutex after modifying the list
        pthread_mutex_unlock(&receiveListMutex);


        if (strcmp(inputMessage, "!") == 0){
            printf("3. input thread should shutdown...\n");
            pthread_cond_signal(&sendListNotEmptyCond);
            pthread_cond_signal(&receiveListNotEmptyCond); 
            signal_shutdown();
            return NULL;
        }

    }

    close(sockfd);
    return NULL;
}

void* screenOutputThread(void* args) {

    // thread_args* arguments = (thread_args*)args;

    while (1) {

        // Lock the mutex before accessing the shared list
        pthread_mutex_lock(&receiveListMutex);
    
        // Wait for the list to have messages
        while (receiveList->head == NULL) {

            if (should_shutdown()){
                printf("1. screen thread should shutdown...\n");
                return NULL;
            }

            pthread_cond_wait(&receiveListNotEmptyCond, &receiveListMutex);
        }

        // Assuming list_remove returns a dynamically allocated string
        // that needs to be freed after use
        char* message = List_trim(receiveList);

        // Unlock the mutex after removing the message from the list
        pthread_mutex_unlock(&receiveListMutex);

        // Display the message
        if (message != NULL) {
            printf("%s\n", message);

            if (should_shutdown()){
                printf("2. screen thread should shutdown...\n");
                free(message);
                return NULL;
            }

            free(message);
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
    printf("┍――――――――――――――――――――――――――――――――――┑\n");
    printf("|   Local port: %s\n", argv[1]);
    printf("|   Remote IP: %s\n", argv[2]);
    printf("|   Remote port: %s\n", argv[3]);
    printf("┕――――――――――――――――――――――――――――――――――┙\n");
    printf("Press ! to exit\n");
    printf("start of chat:\n");

    init_shutdown_manager();

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
    // args->sendList = List_create();
    // args->receiveList = List_create();

    sendList = List_create();
    receiveList = List_create();


    // Create threads
    pthread_create(&threads[0], NULL, keyboardInputThread, NULL);
    pthread_create(&threads[1], NULL, udpOutputThread, args);
    pthread_create(&threads[2], NULL, udpInputThread, args);
    pthread_create(&threads[3], NULL, screenOutputThread, args);


    waitFor_shutdown();

    //cleanup
     
    pthread_cancel(threads[0]);
    // pthread_cancel(threads[1]);
    pthread_cancel(threads[2]);
    // pthread_cancel(threads[3]);

    printf("Exiting...\n");

    free(keyboardMessage);
    free(inputMessage);

    // Wait for threads to finish
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    

    cleanup_shutdown_manager();

    pthread_mutex_destroy(&sendListMutex);
    pthread_mutex_destroy(&receiveListMutex);
    pthread_cond_destroy(&sendListNotEmptyCond);
    pthread_cond_destroy(&receiveListNotEmptyCond);

    List_free(sendList, freeList);
    List_free(receiveList, freeList);

    free(args);

    return 0;
}