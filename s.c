#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define PKT_SIZE 65000
#define SPL_PKT "END_OF_TRANSM"

int main(){
	int serverSocket;
	struct sockaddr_in serverAddr={0},clientAddr={0};
	socklen_t addrLen = sizeof(struct sockaddr);
	
	if((serverSocket = socket(AF_INET,SOCK_DGRAM,0)) < 0){
		perror("Socket Creation failed");
		exit(1);
	}
	printf("SOcket Created Successfully\n");
	memset(&serverAddr,0,sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(9090);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	printf("Port 9090 paired with new socket\n");
	if (bind(serverSocket,(struct sockaddr*)&serverAddr,sizeof(serverAddr))<0){
		perror("Bind Failed");
		exit(1);
	}
	printf("Bind Successssful\n");
	
	char rcvdPkt[PKT_SIZE];
	bzero(rcvdPkt,PKT_SIZE);
	//for(int j=0;j<2;j++){
	//char filename[20] ;
	//	sprintf(filename, "recv%d.mp4", j);
	//	FILE *fp = fopen(filename,"a");
	FILE *fp = fopen("new_file1.mp4","a");
	if(!fp){
		perror("Error opening video file\n");
		exit(1);
	}
	printf("Server is waiting to receive packets..\n");
	int pknum=0;
	while(1){
		printf("Looping\n");
		ssize_t rcvdBytes = recvfrom(serverSocket,rcvdPkt , PKT_SIZE,0,(struct sockaddr*)&clientAddr , &addrLen);
		char clientIp[256] ;
		bzero(clientIp,256);
		printf("Client IP: %s\n",clientIp);
		if (rcvdBytes <0){
			perror("Packet Lost\n");
		}
		inet_ntop(AF_INET,&clientAddr.sin_addr,clientIp,256);
		printf("Packet %d received\n",++pknum);
		printf("%s\n",rcvdPkt);
		
		//char first[13]=SPL;
		int flag = 1;
		for(int i=0;i<13;i++){			
			if(SPL_PKT[i] != rcvdPkt[i]){
				flag = 0;
				break;
			}
		}
		if (flag){
			printf("End of transmission packet received\n");
			break;
		}
		fwrite(rcvdPkt ,rcvdBytes,1,fp);
		sleep(0.2);
	}	
	fclose(fp);
	printf("New file created\n");
	//}
	close(serverSocket);
	printf("Server scoket closed\n");
	return 0;
}
