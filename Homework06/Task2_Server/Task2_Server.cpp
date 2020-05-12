// Task2_Server.cpp : Defines the entry point for the console application.
//
// SelectTCPServer.cpp : Defines the entry point for the console application.
//

#ifndef FD_SETSIZE
#define FD_SETSIZE 1024
#endif

#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <stdio.h>
#include <conio.h>
#include <WS2tcpip.h>

#pragma comment (lib,"ws2_32.lib")

#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048

int Receive(SOCKET, char *, int, int);
int Send(SOCKET, char *, int, int);
int checkIp(char *);
void ipToDomain(char *, char *);
void domainToIp(char *, char *);

int main(int argc, char* argv[])
{
	if (argc != 2) {
		printf("Not found parameter or to much parameter");
		return 0;
	}
	//Get server port from input parameters
	int	SERVER_PORT = atoi(argv[1]);

	//Step 1: Initiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))
		printf("Version is not supported\n");

	//Step 2: Construct socket	
	SOCKET listenSock;
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Step 3: Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error! Cannot bind this address.");
		_getch();
		return 0;
	}

	//Step 4: Listen request from client
	if (listen(listenSock, 10)) {
		printf("Error! Cannot listen.");
		_getch();
		return 0;
	}

	printf("Server started!\n");

	SOCKET client[FD_SETSIZE], connSock;
	fd_set readfds, initfds; //use initfds to initiate readfds at the begining of every loop step
	sockaddr_in clientAddr;
	int ret, nEvents, clientAddrLen;
	char rcvBuff[BUFF_SIZE], sendBuff[BUFF_SIZE];

	for (int i = 0; i < FD_SETSIZE; i++)
		client[i] = 0;	// 0 indicates available entry

	FD_ZERO(&initfds);
	FD_SET(listenSock, &initfds);

	//Step 5: Communicate with clients
	while (1) {
		readfds = initfds;		/* structure assignment */
		nEvents = select(0, &readfds, 0, 0, 0);
		if (nEvents < 0) {
			printf("\nError! Cannot poll sockets: %d", WSAGetLastError());
			break;
		}

		//new client connection
		if (FD_ISSET(listenSock, &readfds)) {
			clientAddrLen = sizeof(clientAddr);
			if ((connSock = accept(listenSock, (sockaddr *)&clientAddr, &clientAddrLen)) < 0) {
				printf("\nError! Cannot accept new connection: %d", WSAGetLastError());
				break;
			}
			else {
				printf("You got a connection from %s\n", inet_ntoa(clientAddr.sin_addr)); /* prints client's IP */

				int i;
				for (i = 0; i < FD_SETSIZE; i++)
					if (client[i] == 0) {
						client[i] = connSock;
						FD_SET(client[i], &initfds);
						break;
					}

				if (i == FD_SETSIZE) {
					printf("\nToo many clients.");
					closesocket(connSock);
				}

				if (--nEvents == 0)
					continue; //no more event
			}
		}

		//receive data from clients
		for (int i = 0; i < FD_SETSIZE; i++) {
			if (client[i] == 0)
				continue;

			if (FD_ISSET(client[i], &readfds)) {
				ret = Receive(client[i], rcvBuff, BUFF_SIZE, 0);
				if (ret <= 0) {
					FD_CLR(client[i], &initfds);
					closesocket(client[i]);
					client[i] = 0;
				}
				else if (ret > 0) {
					rcvBuff[ret] = 0;
					if (checkIp(rcvBuff))
						ipToDomain(rcvBuff, sendBuff);
					else
						domainToIp(rcvBuff, sendBuff);
					Send(client[i], sendBuff, strlen(sendBuff), 0);
				}
			}

			if (--nEvents <= 0)
				continue; //no more event
		}

	}

	closesocket(listenSock);
	WSACleanup();
	return 0;
}

/* The recv() wrapper function */
int Receive(SOCKET s, char *buff, int size, int flags) {
	int n;
	n = recv(s, buff, size, flags);
	if (n == SOCKET_ERROR)
		printf("Error: %", WSAGetLastError());

	return n;
}

/* The send() wrapper function*/
int Send(SOCKET s, char *buff, int size, int flags) {
	int n;

	n = send(s, buff, size, flags);
	if (n == SOCKET_ERROR)
		printf("Error: %", WSAGetLastError());

	return n;
}

int checkIp(char *input) {
	//Count dot of input
	int i, count = 0;
	for (i = 0; input[i] != NULL; i++) {
		if (input[i] == '.')
			count++;
	}

	//Check number of dot
	if (count != 3)
		return 0;
	//Use inet_addr() funtion to check ip valide
	else if (inet_addr(input) == INADDR_NONE)
		return 0;
	else
		return 1;
}

void ipToDomain(char *input, char *resultDomain) {
	//Declare and initialize variables
	WSADATA wsaData = { 0 };
	int iResult = 0;
	struct in_addr addr = { 0 };
	struct hostent *remoteHost;
	char **pAlias;

	//Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return;
	}
	addr.s_addr = inet_addr(input);

	//get domain
	remoteHost = gethostbyaddr((char *)&addr, 4, AF_INET);
	if (WSAGetLastError() == WSANO_DATA) {
		strcpy(resultDomain, "Not found infomation!\n");
		return;
	}
	strcpy(resultDomain, "\nOfficial Name:\n\t");
	strcat(resultDomain, remoteHost->h_name);

	//check alias name
	if (remoteHost->h_aliases != NULL) {
		strcat(resultDomain, "\n");
		if (*remoteHost->h_aliases != NULL) {
			strcat(resultDomain, "Alias Name:\n\t");
			for (pAlias = remoteHost->h_aliases; *pAlias != 0; pAlias++) {
				strcat(resultDomain, "\t");
				strcat(resultDomain, *pAlias);
				strcat(resultDomain, "\n");
			}
		}
	}
}

void domainToIp(char *input, char *resultIp) {
	// Declare and initialize variables
	WSADATA wsaData;
	INT iResult;
	DWORD dwRetval;
	char* HOST_NAME = input;

	struct addrinfo *result = NULL, *rp;
	struct addrinfo *ptr = NULL;
	struct addrinfo hints;
	struct sockaddr_in  *sockaddr_ipv4;
	LPSOCKADDR sockaddr_ip;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		printf("Error code: %d", WSAGetLastError());
		return;
	}

	//--------------------------------
	// Setup the hints address info structure
	// which is passed to the getaddrinfo() function
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//--------------------------------
	// Call getaddrinfo(). If the call succeeds,
	// the result variable will hold a linked list
	// of addrinfo structures containing response
	// information
	dwRetval = getaddrinfo(HOST_NAME, "http", &hints, &result);

	if (dwRetval != 0) {
		strcpy(resultIp, "Not found infomation!");
		WSACleanup();
		return;
	}

	//Retrieve list address
	strcpy(resultIp, "\nOfficial IP: \n\t");
	sockaddr_ipv4 = (struct sockaddr_in *)result->ai_addr;
	strcat(resultIp, inet_ntoa(sockaddr_ipv4->sin_addr));
	if (result->ai_next != NULL) {
		strcat(resultIp, "\nAlias IP: \n\t");
		for (rp = result->ai_next; rp != NULL; rp = rp->ai_next) {
			sockaddr_ipv4 = (struct sockaddr_in *) rp->ai_addr;
			strcat(resultIp, inet_ntoa(sockaddr_ipv4->sin_addr));
			strcat(resultIp, "\n\t");
		}
	}

	freeaddrinfo(result);
}