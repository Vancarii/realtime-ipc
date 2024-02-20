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

List sendList;
pthread_mutex_t sendListMutex;
pthread_cond_t sendListNotEmptyCond;

List receiveList;
pthread_mutex_t receiveListMutex;
pthread_cond_t receiveListNotEmptyCond;

typedef struct {
    char *remoteIP;
    int remotePort;
    int localPort;
    // List *sendList;
    // List *receiveList;
} thread_args;


void* udpOutputThread(void* args) {
    int sockfd;
    struct sockaddr_in servaddr;

    char* message;

    char inputBuffer[1024]; // Buffer to hold input from the keyboard

    while (fgets(inputBuffer, sizeof(inputBuffer), stdin) != NULL) {
        // Remove newline character at the end of input if present
        inputBuffer[strcspn(inputBuffer, "\n")] = 0;

        // Allocate memory for a new message
        message = strdup(inputBuffer); // Duplicate the string to store in the list

        
        printf("in keyboard input thread---------------------------\n");
        printf("keyboard input: %s\n", message);

  

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

    // Send the message
    if (message != NULL) {
        sendto(sockfd, message, strlen(message), 0, (const struct sockaddr*)&servaddr, sizeof(servaddr));
        free(message); // Don't forget to free the memory
    }
    

    close(sockfd);

    }

    return NULL;
}

void* udpInputThread(void* args) {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    
    // char* message;

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


        printf("in screen output thread---------------------------\n");
        printf("screen output: %s\n", buffer);

    }

    close(sockfd);
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

    thread_args *args = malloc(sizeof *args);
    args->localPort = atoi(argv[1]);
    args->remoteIP = argv[2];
    args->remotePort = atoi(argv[3]);

    pthread_t threads[2];


    // Create threads
    pthread_create(&threads[0], NULL, udpOutputThread, args);
    pthread_create(&threads[1], NULL, udpInputThread, args);

    // Wait for threads to finish
    for (int i = 0; i < 2; i++) {
        pthread_join(threads[i], NULL);
    }

    // Cleanup
    return 0;
}