#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 65000
#define SPECIAL_PACKET "END_OF_TRANSMISSION"

int main() {
    int clientSockets[5];
    struct sockaddr_in serverAddr;
    socklen_t addrLen = sizeof(struct sockaddr);

    // Create sockets for each client
    for (int i = 0; i < 5; i++) {
        clientSockets[i] = socket(AF_INET, SOCK_DGRAM, 0);
        if (clientSockets[i] == -1) {
            perror("Error creating client socket");
            exit(1);
        }
    }

    // Configure the server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9090);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Open and read the video file for each client
    FILE *inputFiles[5];
    for (int i = 0; i < 5; i++) {
        char filename[10];
        sprintf(filename, "v%d.mp4", i);
        inputFiles[i] = fopen(filename, "rb");
        if (inputFiles[i] == NULL) {
            perror("Error opening the input file");
            for (int j = 0; j < i; j++) {
                fclose(inputFiles[j]);
            }
            for (int j = 0; j < 5; j++) {
                close(clientSockets[j]);
            }
            exit(1);
        }
    }

    char sendBuffer[MAX_BUFFER_SIZE];
    int bytesRead;
    int packetCount = 0;
    while (1) {
        for (int i = 0; i < 5; i++) {
            bytesRead = fread(sendBuffer, 1, sizeof(sendBuffer), inputFiles[i]);
            if (bytesRead <= 0) {
                continue; // No more data to send for this client
            }
            ssize_t sentBytes = sendto(clientSockets[i], sendBuffer, bytesRead, 0,
                (struct sockaddr *)&serverAddr, addrLen);
            if (sentBytes == -1) {
                perror("Error sending data to server");
                for (int j = 0; j < 5; j++) {
                    fclose(inputFiles[j]);
                    close(clientSockets[j]);
                }
                exit(1);
            }
            printf("Client %d: Packet %d transmitted successfully: %zd bytes\n", i, ++packetCount, sentBytes);
        }
    }

    // Send the special packet to signal the end of transmission for each client
    for (int i = 0; i < 5; i++) {
        ssize_t sentBytes = sendto(clientSockets[i], SPECIAL_PACKET, strlen(SPECIAL_PACKET), 0,
            (struct sockaddr *)&serverAddr, addrLen);
        if (sentBytes == -1) {
            perror("Error sending the end-of-transmission packet to server");
            for (int j = 0; j < 5; j++) {
                fclose(inputFiles[j]);
                close(clientSockets[j]);
            }
            exit(1);
        }
        printf("Client %d: End-of-transmission packet sent successfully.\n", i);
    }

    for (int i = 0; i < 5; i++) {
        fclose(inputFiles[i]);
        close(clientSockets[i]);
    }

    return 0;
}
