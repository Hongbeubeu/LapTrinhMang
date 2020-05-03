// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 20048

#pragma comment(lib, "Ws2_32.lib")

int checkIp(char *input);
void ipToDomain(char *input, char *resultDomain);
void domainToIp(char *input, char *resultIp);
int main(int argc, char *argv[])
{
	//Check input argument
	if (argc != 2) {
		printf("Not found port number\n");
		return 0;
	}

	u_short SERVER_PORT = atoi(argv[1]);

	//Step 1: Inittiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))
		printf("Version is not supported\n");
	//step2: Construct socket
	SOCKET server;
	server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	//step 3: Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	if (bind(server, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		printf("Error! Cannot bind this address.");
		_getch();
		return 0;
	}
	printf("Server started!");

	//Step 4: Communicate with client
	sockaddr_in clientAddr;
	char buff[BUFF_SIZE];
	char result[BUFF_SIZE];
	int ret, clientAddrLen = sizeof(clientAddr);

	while (true) {
		//Receive message
		ret = recvfrom(server, buff, BUFF_SIZE, 0, (sockaddr *)&clientAddr, &clientAddrLen);

		if (ret == SOCKET_ERROR)
			printf("Error: %d", WSAGetLastError());
		else if (strlen(buff) > 0) {
			buff[ret] = 0;

			//Check client send ip or domain
			if (checkIp(buff))
				ipToDomain(buff, result);
			else
				domainToIp(buff, result);

			//Echo to client
			ret = sendto(server, result, strlen(result), 0, (SOCKADDR *)&clientAddr, sizeof(clientAddr));
			if (ret == SOCKET_ERROR)
				printf("Error: %d", WSAGetLastError());
		}// end while
	}
	return 0;
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
	strcpy(resultDomain, "Official Name:\n\t");
	strcat(resultDomain, remoteHost->h_name);

	//check alias name
	if (remoteHost->h_aliases != NULL) {
		strcat(resultDomain, "\n");
		strcat(resultDomain, "Alias Name:\n\t");
		for (pAlias = remoteHost->h_aliases; *pAlias != 0; pAlias++) {
			strcat(resultDomain, "\t");
			strcat(resultDomain, *pAlias);
			strcat(resultDomain, "\n");
		}
	}
	WSACleanup();
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
	strcat(resultIp, "\nAlias IP: \n\t");
	for (rp = result->ai_next; rp != NULL; rp = rp->ai_next) {
		sockaddr_ipv4 = (struct sockaddr_in *) rp->ai_addr;
		strcat(resultIp, inet_ntoa(sockaddr_ipv4->sin_addr));
		strcat(resultIp, "\n\t");
	}

	freeaddrinfo(result);

	WSACleanup();
}

