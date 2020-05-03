// TCPServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#define SERVER_PORT 5500
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
	
#pragma comment(lib, "Ws2_32.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))
		printf("Version is not supported\n");

	SOCKET listenSock;
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	if (bind(listenSock, (sockaddr *) &serverAddr, sizeof(serverAddr)))
	{
		printf("Error! Cannot bind this address.");
		_getch();
		return 0;
	}

	if (listen(listenSock, 10)) {
		printf("Error! Cannot listen.");
		_getch();
		return 0;
	}

	printf("Server started!");

	sockaddr_in clientAddr;
	char buff[BUFF_SIZE];
	int ret, clientAddrlLen = sizeof(clientAddr);

	while (1)
	{
		SOCKET connSock;

		connSock = accept(listenSock, (sockaddr *)&clientAddr, &clientAddrlLen);

		ret = recv(connSock, buff, BUFF_SIZE, 0);
		if(ret == SOCKET_ERROR) {
			printf("Error : %d", WSAGetLastError());
			break;
		}
		else if (strlen(buff) > 0) {
			buff[ret] = 0;
			printf("Receive from client [%s: %d] %s \n",
				inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), buff);

			ret = send(connSock, buff, strlen(buff), 0);
			if (ret == SOCKET_ERROR)
				printf("Error: %d", WSAGetLastError());
		}
		closesocket(connSock);
	}

	closesocket(listenSock);

	WSACleanup();
    return 0;
}

