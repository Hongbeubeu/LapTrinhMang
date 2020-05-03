// UDPEcho.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <conio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#define BUFF_SIZE 20048

#pragma comment(lib, "Ws2_32.lib")
int main(int argc, char *argv[])
{
	u_short SERVER_PORT = atoi(argv[2]);
	char* SERVER_ADDR = argv[1];
	//step1: init winsock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf("Version is not supported.\n");
	}
	printf("Client Started!");

	//step2: construct socket
	SOCKET client;
	client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int tv = 10000; //timeout 1000ms
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)(&tv), sizeof(int));

	//step3: Specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	//step4: communicate with server
	char buff[BUFF_SIZE];
	int ret, serverAddrLen = sizeof(serverAddr);
	do {
		//send message
		printf("Ip or domain: ");
		gets_s(buff, BUFF_SIZE);
		ret = sendto(client, buff, strlen(buff), 0, (sockaddr *)&serverAddr, serverAddrLen);
		if (ret == SOCKET_ERROR)
		{
			printf("Error! Cannot send message.");
		}
		//receive echo message
		ret = recvfrom(client, buff, BUFF_SIZE, 0, (sockaddr *)&serverAddr, &serverAddrLen);
		if (ret == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT)
				printf("Time-out!");
			else printf("Error! Cannot receive message. %d", WSAGetLastError());
		}
		else if (strlen(buff) > 0) {
			buff[ret] = 0;
			printf("Receive from server[%s:%d] %s \n",
				inet_ntoa(serverAddr.sin_addr),
				ntohs(serverAddr.sin_port),
				buff);
		}
		_strupr_s(buff, BUFF_SIZE);
	} while (strcmp(buff, "") != 0);
	//step 5: close socket
	closesocket(client);
	//step 6: terminate winsock
	WSACleanup();
	return 0;
}

