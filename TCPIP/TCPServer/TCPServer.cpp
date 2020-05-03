// TCPClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>	
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#define SERVER_ADDRF "127.0.0.1"
#define BUFF_SIZE 2048
#pragma comment(lib, "Ws2_32.lib")


int main(int argc, char* argv[])
{
	//Verificate parameter
	if (argc != 2) {
		printf("Not found parameter or to much parameter");
		return 0;
	}
	//Get port number
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
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDRF);

	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		printf("Error! Cannot bind this address.");
		_getch();
		return 0;
	}

	//step 4: Listen request from client
	if (listen(listenSock, 10)) {
		printf("Error! Cannot listen.");
		_getch();
		return 0;
	}
	
	printf("Server started");

	//Step 5: Communicate with client
	sockaddr_in clientAddr;

	//initiate parameter
	char buff[BUFF_SIZE],len[BUFF_SIZE], receive[BUFF_SIZE];
	int ret, clientAddrLen = sizeof(clientAddr), length;
	SOCKET connSock;

	//accept request
	connSock = accept(listenSock, (sockaddr *)& clientAddr, &clientAddrLen);

	while (1){
		//receive message from client
		//receive length of message from client
		ret = recv(connSock, len, BUFF_SIZE, 0);
		if (ret == SOCKET_ERROR) {
			printf("Error : %d", WSAGetLastError());
			closesocket(connSock);
			break;
		}
		//Get message
		length = atoi(len);
		buff[0] = 0;
		do {
			//Receive message from client
			ret = recv(connSock	, receive, BUFF_SIZE, 0);
			if (ret == SOCKET_ERROR) {
					printf("Error! Cannot receive message. ");
			}
			else if (strlen(receive) > 0) {
				receive[ret] = 0;
				strcat(buff, receive);
				length -= ret;
			}
		} while (length > 0);//get message until number of byte of message return zero
		
		//Send message to client
		if (strlen(buff) > 0) {
			char sizeMsg[BUFF_SIZE];
			bool isError = false;
			buff[ret] = 0;
			printf("Receive from client[%s:%d] %s\n",
				inet_ntoa(clientAddr.sin_addr),
				ntohs(clientAddr.sin_port),
				buff);
			//check if input contain number
			for (int i = 0; i < ret; i++)
				if (buff[i] >= 48 && buff[i] <= 57) {
					isError = true;
					strcpy(buff, "Error: String contains number.");
					ret = strlen(buff);

					//send length of message to client
					ret = send(connSock, itoa(ret, sizeMsg, 10), ret, 0);
					if (ret == SOCKET_ERROR)
						printf("Error: %d", WSAGetLastError());

					//send message to client
					ret = send(connSock, buff, strlen(buff), 0);
					if (ret == SOCKET_ERROR)
						printf("Error: %d", WSAGetLastError());
					break;
				}

			if (isError == false) {
				//initiate variable for couting number of word
				int counter = 1;
				//Format input
				//Remove leading space
				while (buff[ret - 1] == ' ') {
					ret--;
					buff[ret] = 0;
				}

				//Remove trailing space
				while (buff[0] == ' ') {
					for (int i = 0; i < ret; i++) {
						buff[i] = buff[i + 1];
					}
					ret--;
				}

				//Remove double space
				int i = 0;
				while (buff[i] != 0) {
					if (buff[i] == ' ' && buff[i + 1] == ' ') {
						for (int j = i; j < ret; j++) {
							buff[j] = buff[j + 1];
						}
						ret--;
					}
					else
						i++;
				}

				//Counting number of words
				for (int i = 0; i < ret; i++)
					if (buff[i] == ' ')
						counter++;
				strcpy(buff, "Number of words: ");
				strcat(buff, itoa(counter, sizeMsg, 10));
				ret = strlen(buff);

				//Send length of message to client
				ret = send(connSock, itoa(ret, sizeMsg, 10), ret, 0);
				if (ret == SOCKET_ERROR)
					printf("Error: %d", WSAGetLastError());
				
				//Send message to client
				ret = send(connSock, buff, strlen(buff), 0);
				if (ret == SOCKET_ERROR)
					printf("Error: %d", WSAGetLastError());
			}
		}
	}//end accepting
	

	//Step 6: Close socket
	closesocket(connSock);
	closesocket(listenSock);

	//Step 7: Terminate Winsock
	WSACleanup();

	return 0;
}

