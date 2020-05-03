// Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>	
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#define BUFF_SIZE 2048

#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char* argv[])
{
	//verificate parameter
	if (argc != 3) {
		printf("\nNot found parameter or too much parameter");
		return 0;
	}
	char* SERVER_ADDR = argv[1];
	u_short SERVER_PORT = atoi(argv[2]);

	//Step 1: Inittiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))
		printf("Version is not supported\n");

	//Step 2: Construct socket
	SOCKET client;
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Step 3: Specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);


	//step 4: Request to connect server
	if (connect(client, (sockaddr *)& serverAddr, sizeof(serverAddr))) {
		printf("Error! Cannot connect server. %d", WSAGetLastError());
		_getch();
		return 0;
	}
	printf("Connected server!\n");

	//Step 5: Communicate with server
	char buff[BUFF_SIZE];
	int ret;
	char len[BUFF_SIZE], result[BUFF_SIZE], receive[BUFF_SIZE], sizeMsg[BUFF_SIZE];
	do {
		//Send message to server
		printf("Send to server: ");
		gets_s(buff, BUFF_SIZE);
		ret = strlen(buff);

		//Send length of message to server
		ret = send(client, itoa(ret, sizeMsg, 10), ret, 0);
		if (ret == SOCKET_ERROR)
			printf("Error! Cannot send message. %d\n", WSAGetLastError());

		//Send message to server
		ret = send(client, buff, strlen(buff), 0);
		if (ret == SOCKET_ERROR)
			printf("Error! Cannot send message. %d\n", WSAGetLastError());

		//Get length of message from server
		ret = recv(client, len, BUFF_SIZE, 0);
		int length = atoi(len);
		result[0] = 0;

		do {
			//Receive message from server
			ret = recv(client, receive, BUFF_SIZE, 0);
			if (ret == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					printf("Time-out!");
					break;
				}
				else {
					printf("Error! Cannot receive message. ");
					break;
				}
			}
			else if (strlen(receive) > 0) {
				receive[ret] = 0;
				strcat(result, receive);
				length -= ret;
			}
		} while (length > 0);//get message until number of byte of message return zero

							 //print result
		printf("Receive from server[%s:%d] %s\n",
			inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port), result);
	} while (strcmp(buff, "") != 0);//Close when input is null.

									//Step 6: Close socket
	closesocket(client);

	//Step 7: Terminate Winsock
	WSACleanup();

	return 0;
}

