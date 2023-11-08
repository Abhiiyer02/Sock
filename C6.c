#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h> 

#define MAX_BUFFER_SIZE 65000
#define SPECIAL_PACKET "END_OF_TRANSMISSION"
#define HELLO_MESSAGE "HELLO"

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;
    socklen_t addrLen = sizeof(struct sockaddr);

    // Create a socket
    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == -1) {
        perror("Error creating client socket");
        exit(1);
    }

    // Configure the server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9090);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Open and read the video file
    FILE *inputFile = fopen("v.mp4", "rb");
    if (inputFile == NULL) {
        perror("Error opening the input file");
        close(clientSocket);
        exit(1);
    }

    // Send the "HELLO" message to the server to establish the connection
    ssize_t sentBytes = sendto(clientSocket, HELLO_MESSAGE, strlen(HELLO_MESSAGE), 0,
        (struct sockaddr *)&serverAddr, addrLen);

    if (sentBytes == -1) {
        perror("Error sending 'HELLO' message to server");
        close(clientSocket);
        exit(1);
    }

    char sendBuffer[MAX_BUFFER_SIZE];
    int bytesRead;
    int packetCount = 0;
    while (!feof(inputFile)) {
        // Send a packet to the server
        bytesRead = fread(sendBuffer, 1, sizeof(sendBuffer), inputFile);

        if (bytesRead < 0) {
            printf("Error reading packet %d\n", ++packetCount);
            continue;
        }
        sentBytes = sendto(clientSocket, sendBuffer, bytesRead, 0,
            (struct sockaddr *)&serverAddr, addrLen);

        if (sentBytes == -1) {
            perror("Error sending data to server");
            close(clientSocket);
            exit(1);
        }

        printf("Packet %d transmitted successfully: %zd bytes\n", ++packetCount, sentBytes);
    }

    // Send the special packet to signal the end of transmission
    sentBytes = sendto(clientSocket, SPECIAL_PACKET, strlen(SPECIAL_PACKET), 0,
        (struct sockaddr *)&serverAddr, addrLen);

    if (sentBytes == -1) {
        perror("Error sending the end-of-transmission packet to server");
        close(clientSocket);
        exit(1);
    }
    printf("End-of-transmission packet sent successfully.\n");
    fclose(inputFile);
    close(clientSocket);
    return 0;
}

