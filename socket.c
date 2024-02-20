#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MSG_MAX_LEN 1024
#define PORT 22110

int main(){
    printf("Yecheng net listen test on UDP port %d\n", PORT);
    printf("Connect using: \n");
    printf("    netcat -u 127.0.0.1 22110\n", PORT);

    //Address
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET; //COnnection mayt be from network
    sin.sin_addr.s_addr = htonl(INADDR_ANY); // Host to network long
    sin.sin_port = htons(PORT); // host to network short

    //create socket
    int socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);

    //Bind
    bind(socketDescriptor, (struct sockaddr *)&sin, sizeof(sin));

    //Receive
    while(1){
        
        struct sockaddr sinRemote;
        unsigned int sin_len = sizeof(sinRemote);
        char messageRx[MSG_MAX_LEN];
        int bytesRx = recvfrom(socketDescriptor, 
            messageRx, MSG_MAX_LEN, 0, 
            (struct sockaddr *) &sinRemote, &sin_len);

        // make it null terminated
        int terminateIdx = (bytesRx < MSG_MAX_LEN) ? bytesRx : MSG_MAX_LEN - 1;
        messageRx[terminateIdx] = 0;
        printf("%s\n", messageRx);

        // int incMe = atoi(messageRx);

        // char messageTx[MSG_MAX_LEN];
        // sprintf(messageTx, "math %d + 1 = %d\n", incMe, incMe + 1);

        // sin_len = sizeof(sinRemote);
        // sendto(socketDescriptor, 
        // messageTx, strlen(messageTx),
        //  0, 
        //  (struct sockaddr *) &sinRemote, sin_len);
    
    }

    //close 
    close(socketDescriptor);
    return 0;

}

// int main(int argc, char *argv[]) {
//     int sockfd;
//     struct sockaddr_in serverAddr, clientAddr;
//     char buffer[MSG_MAX_LEN];
//     int addrLen = sizeof(struct sockaddr_in);
//     int recvLen;

//     // Create socket
//     if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
//         perror("socket");
//         exit(1);
//     }

//     // Set server address
//     memset(&serverAddr, 0, sizeof(serverAddr));
//     serverAddr.sin_family = AF_INET;
//     serverAddr.sin_port = htons(PORT);
//     serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

//     // Bind socket to server address
//     if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
//         perror("bind");
//         exit(1);
//     }

//     // Receive message
//     while (1) {
//         recvLen = recvfrom(sockfd, buffer, MSG_MAX_LEN, 0, (struct sockaddr *)&clientAddr, &addrLen);
//         if (recvLen < 0) {
//             perror("recvfrom");
//             exit(1);
//         }
//         buffer[recvLen] = 0;
//         printf("Received: %s\n", buffer);
//     }

//     return 0;
// }
// ```

// ```c