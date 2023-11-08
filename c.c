#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX 65000
#define SPL_PKT "END_OF_TRANSM"

int main(){
	int clientSocket;
	struct sockaddr_in serverAddr={0};
	socklen_t addrLen = sizeof(struct sockaddr);
	
	if ((clientSocket = socket(AF_INET,SOCK_DGRAM,0))<0){
		perror("Client Socket Creatin failed");
		exit(1);
	}
	printf("Socket Created successfully");
	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(9090);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//for(int j =1;j<=1;j++){
	//char filename[10] ;
	//	sprintf(filename, "send%d.mp4", j);
//		printf("%s",filename);
//		FILE *fp = fopen(filename,"a");
	
	FILE *fp = fopen("send1.mp4","r");
	if (fp == NULL){
		perror("Error opening file");
		exit(1);
	}
	printf("File opened successfully , reading file...\n");
	
	char sendBuffer[MAX];
	bzero(sendBuffer,MAX);
	int pktCount=0 ;
	size_t bytesRead ;
	while(!feof(fp)){
		++pktCount;
		if(bytesRead = fread(sendBuffer,1,MAX,fp)<0){
			perror("File chunk %d cannot be read\n");
			continue;
		}
		printf("%ld\n",bytesRead);
		ssize_t sentBytes = sendto(clientSocket,sendBuffer,65000,0,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
		if(sentBytes<0){
			perror("packet  transmission failed\n");
			exit(1);
		}
		printf("Packet %d sent \n",pktCount);
		printf("Packet contents:\n%s\n",sendBuffer);
		//printf("%s",sendBuffer);		
		sleep(1);
	}
	ssize_t lastTransm = sendto(clientSocket,SPL_PKT,strlen(SPL_PKT),0,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
	if (lastTransm<0){
		perror("Error in sending last packet\n");
	}
	printf("Closing Video file\n");
	fclose(fp);
	//}
	printf("Closing Socket..\n");
	close(clientSocket);
	return 0 ;
}
