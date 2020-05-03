// AddressResolver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define UNICODE
#include <tchar.h>
#include <WinSock2.h>
#include <stdio.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <string.h>

#pragma comment(lib, "Ws2_32.lib")

//checkIp return 1 if Ip valide else return 0
int checkIp(char *input);
void ipToDomain(char *input);
void domainToIp(char *input);

int main(int argc, char *argv[]){
	//check number of agrument
	if (argc != 2){
		printf("Error! No agrument or much argument\n");
		return 0;
	}
	char *input;
	input = argv[1];

	//check valide ip
	if (checkIp(input))
		ipToDomain(input);
	else
		domainToIp(input);
    return 0;
}

int checkIp(char *input) {
	//count dot of input
	int i, count = 0;
	for (i = 0; input[i] != NULL; i++) {
		if (input[i] == '.')
			count++;
	}

	//check number of dot
	if (count != 3)
		return 0;
	//use inet_addr() funtion to check ip valide
	else if (inet_addr(input) == INADDR_NONE)
		return 0;
	else
		return 1;
}

void ipToDomain(char *input) {
	
	// Declare and initialize variables
	WSADATA wsaData = { 0 };
	int iResult = 0;

	//this variables use for getnameinfo()
	/*DWORD dwRetval;
	struct sockaddr_in saGNI;
	char hostname[NI_MAXHOST];
	char servInfo[NI_MAXSERV];
	u_short port = 27015;*/

	//this variables use for getaddrbyname()
	struct in_addr addr = { 0 };
	struct hostent *remoteHost;
	char **pAlias;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return;
	}

	//this is use for getnameinfo()
	//-----------------------------------------
	// Set up sockaddr_in structure which is passed
	// to the getnameinfo function
	//saGNI.sin_family = AF_INET;
	//saGNI.sin_addr.s_addr = inet_addr(input);
	//saGNI.sin_port = htons(port);

	////-----------------------------------------
	//// Call getnameinfo
	//dwRetval = getnameinfo((struct sockaddr *) &saGNI,
	//	sizeof(struct sockaddr),
	//	hostname,
	//	NI_MAXHOST, servInfo, NI_MAXSERV, NI_NUMERICSERV);

	////getnameinfo return hostname as ip address if not found domain match with input ip
	////use inet_addr() to check if hostname return an ip address. Then print not found info with input ip
	////because if hostname is not an ip then inet_addr(hostname) will return INADDR_NONE
	//if (dwRetval != 0 || inet_addr(hostname) != INADDR_NONE){
	//	printf("Not found infomation");
	//	WSACleanup();
	//	return;
	//}
	//else {
	//	printf("Host name: \n	%s\n", hostname);
	//	return;
	//}
	addr.s_addr = inet_addr(input);
	remoteHost = gethostbyaddr((char *)&addr, 4, AF_INET);

	//No data record of requested type.
	if (WSAGetLastError() == WSANO_DATA) {
		printf("Not found infomation");
		return;
	}
	printf("List of name:\n");
	printf("\t%s\n", remoteHost->h_name);
	for (pAlias = remoteHost->h_aliases; *pAlias != 0; pAlias++) {
		printf("\t%s\n", *pAlias);
	}
	WSACleanup();

}

void domainToIp(char *input) {
	
	//-----------------------------------------
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
		printf("Not found infomation!");
		WSACleanup();
		return;
	}

	// Retrieve list address and print out
	printf("List of Ips: \n");
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sockaddr_ipv4 = (struct sockaddr_in *) rp->ai_addr;
		printf("\tIPv4 address %s\n", inet_ntoa(sockaddr_ipv4->sin_addr));
	}

	freeaddrinfo(result);

	WSACleanup();
}