#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>  // Include pthread library for multi-threading

#define PACKET_SIZE 65000
#define SPECIAL_PACKET "END_OF_TRANSMISSION"

void *handleClient(void *arg) {
    int clientSocket = *((int *)arg);
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(struct sockaddr);

    // Prepare a buffer to store incoming data for each packet
    char receivedPacket[PACKET_SIZE];
    int receivedDataSize = 0;

    // Open the output file for each client
    char fileName[20];
    sprintf(fileName, "client_%d_video.mp4", clientSocket);
    FILE *outputFile = fopen(fileName, "ab");
    if (outputFile == NULL) {
        perror("Error opening the output file");
        close(clientSocket);
        pthread_exit(NULL);
    }

    printf("Client %d is connected and waiting for packets...\n", clientSocket);

    while (1) {
        ssize_t receivedBytes = recvfrom(clientSocket, receivedPacket, sizeof(receivedPacket), 0,
            (struct sockaddr *)&clientAddr, &addrLen);

        if (receivedBytes == -1) {
            perror("Error receiving data from client");
            continue;
        }

        // Check if it's the special packet indicating the end of transmission
        if (strcmp(receivedPacket, SPECIAL_PACKET) == 0) {
            printf("Client %d received the special packet. End of transmission.\n", clientSocket);
            break;
        }

        // Write the received data to the output file
        fwrite(receivedPacket, 1, receivedBytes, outputFile);
    }

    // Close the output file after receiving the special packet
    fclose(outputFile);
    printf("Client %d: Output file closed.\n", clientSocket);

    close(clientSocket);
    printf("Client %d: Connection closed.\n", clientSocket);

    pthread_exit(NULL);
}

int main() {
    int serverSocket;
    struct sockaddr_in serverAddr;
    socklen_t addrLen = sizeof(struct sockaddr);

    // Create a socket
    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == -1) {
        perror("Error creating server socket");
        exit(1);
    } else {
        printf("Server socket created successfully.\n");
    }

    // Configure the server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9090);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the server socket to the specified address and port
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error binding server socket");
        close(serverSocket);
        exit(1);
    } else {
        printf("Server socket bound to port 9090 and address INADDR_ANY.\n");
    }

    // Create threads to handle multiple clients
    pthread_t threads[5];  // Assuming a maximum of 5 clients
    int clientSockets[5];
    int clientCount = 0;

    while (1) {
        struct sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(struct sockaddr);

        int newClientSocket;
        ssize_t receivedBytes = recvfrom(serverSocket, NULL, 0, 0, (struct sockaddr *)&clientAddr, &addrLen);

        if (receivedBytes == -1) {
            perror("Error accepting client connection");
            continue;
        }

        // Accept the new client connection and store the socket
        newClientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
        if (newClientSocket == -1) {
            perror("Error accepting client connection");
            continue;
        }

        // Create a thread to handle the new client
        if (clientCount < 5) {
            clientSockets[clientCount] = newClientSocket;
            if (pthread_create(&threads[clientCount], NULL, handleClient, &clientSockets[clientCount]) != 0) {
                perror("Error creating thread");
                close(newClientSocket);
            } else {
                printf("Accepted client %d.\n", newClientSocket);
                clientCount++;
            }
        } else {
            printf("Server is at maximum capacity. Rejecting client connection.\n");
            close(newClientSocket);
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < clientCount; i++) {
        pthread_join(threads[i], NULL);
    }

    close(serverSocket);
    printf("Server socket closed.\n");

    return 0;
}
