#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include "socket.h"


void socket_creation(thread_args* arguments){

    int sockfd;

    struct addrinfo hints;
    struct addrinfo *res;

    struct sockaddr_in servaddr;


    int status;

    // network socket creation
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Filling server information for the socket getaddrinfo send port
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    // Filling server information for the socket in port to receive
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;

    // store the local port in the struct
    servaddr.sin_port = htons(arguments->localPort);

    // Binding the socket to the address
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    char portStr[6];
    sprintf(portStr, "%d", arguments->remotePort);

    if ((status = getaddrinfo(arguments->remoteHostname, portStr, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }


    // store the socket and res in the arguments struct
    // to be passed to the threads
    arguments->socket = sockfd;
    arguments->res = res;

}