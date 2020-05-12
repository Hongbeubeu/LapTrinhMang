// Task1_Server.cpp : Defines the entry point for the console application.
//
// SelectTCPServer.cpp : Defines the entry point for the console application.
//

#ifndef FD_SETSIZE
#define FD_SETSIZE 1024
#endif

#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>	
#include <conio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <process.h>
#include <list>
#include <iostream>

#pragma comment (lib,"ws2_32.lib")

using namespace std;

const char file[] = "account.txt";

#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#define MSG_SIZE 50
#define LOGIN 1
#define LOGOUT 2
#define SIZE 10

struct Message
{
	int type;
	char username[MSG_SIZE];
	char password[MSG_SIZE];
};

struct Account {
	char UserID[MSG_SIZE];
	char Password[MSG_SIZE];
	int Status;
	int passFail;
};

//accounts contain account read in file acounts.txt
struct Account accounts[SIZE];

//session contain account is loged in
list<const char*> session;
int numUsers;

void processData(char *, char *, int);
int Receive(SOCKET, char *, int, int);
int Send(SOCKET, char *, int, int);
bool checkUsername(char *, char *);
bool isActived(char *, char *);
bool checkPassword(char *, char *, char *);
void lockAccount(char *, char *);
void listAccount(char *, int *);
bool checkLogedIn(char *, list<const char*>);
void incPassFail(char *, char *);
int numPassFail(char *, char *);

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
	
	listAccount((char *)accounts, &numUsers); //List account from file

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
					processData(rcvBuff, sendBuff, i);
					Send(client[i], sendBuff, ret, 0);
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

/* The processData function copies the input string to output
* @param in Pointer to input string
* @param out Pointer to output string
* @return No return value
*/
void processData(char *in, char *out, int numClient) {
	struct Message *receiveMessage;
	
	receiveMessage = (struct Message*)in;
	int typeMessage = receiveMessage->type;
	switch (typeMessage) {
		case LOGIN: {
			if (checkUsername(receiveMessage->username, (char*)accounts)) {
				if (!isActived(receiveMessage->username, (char*)accounts)) {
					printf("Server: Account is locked\n");
					strcpy(out, "Server: Account is locked\n");
					break;
				}
				else if (checkLogedIn(receiveMessage->username, session)) {
					printf("Server: This Account is loged in\n");
					strcpy(out, "Server: This Account is loged in\n");
					break;
				}
				else if (checkPassword(receiveMessage->username, receiveMessage->password, (char*)accounts)) {
						printf("Server: Login success\n");
						strcpy(out, "Server: Login success\n");
						session.push_back(receiveMessage->username);
						printf("Hello %s\n", receiveMessage->username);
						break;
					}
					else {
						incPassFail(receiveMessage->username, (char *)accounts);
						if (numPassFail(receiveMessage->username, (char *)accounts) >= 3) {
							printf("Server:This account is locked because try to access into server with incorrect password three times\n");
							strcpy(out, "Server: This account is locked because try to access into server with incorrect password three times\n");
							lockAccount(receiveMessage->username, (char *)accounts);
							listAccount((char *)accounts, &numUsers);
							break;
						}
						else {
							printf("Server: Incorrect password\n");
							strcpy(out, "Server: Incorrect password\n");
							break;
						}
					}
			}
			else {
				printf("Server: Incorrect username\n");
				strcpy(out, "Server: Incorrect username\n");
				break;
			}
		}
		case LOGOUT: {
			if (checkLogedIn(receiveMessage->username, session)) {
				printf("Server: Logout!\n");
				strcpy(out, "Server: Logout!\n");
				session.remove(receiveMessage->username);
				break;
			}
			else {
				printf("Server: You haven't login\n");
				strcpy(out, "Server: You haven't login\n");
				break;
			}
		}
		default: {
			session.remove(receiveMessage->username);
			return;
		}
	}
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
bool checkUsername(char *username, char *accounts) {
	struct Account *acc = (struct Account *)accounts;
	for (int i = 0; i < numUsers; i++) {
		if (strcmp(username, (acc + i)->UserID) == 0)
			return true;
	}
	return false;
}

bool isActived(char *username, char *accounts) {
	struct Account *acc = (struct Account *)accounts;
	for (int i = 0; i < numUsers; i++) {
		if (strcmp(username, (acc + i)->UserID) == 0)
			if ((acc + i)->Status == 0) {
				return true;
			}
	}
	return false;
}
bool checkPassword(char *username, char *password, char *accounts) {
	struct Account *acc = (struct Account *)accounts;
	for (int i = 0; i < numUsers; i++) {
		if (strcmp(username, (acc + i)->UserID) == 0)
			if (strcmp(password, (acc + i)->Password) == 0) {
				return true;
			}
	}
	return false;
}

void lockAccount(char *username, char *accounts) {
	int offset = 0;
	struct Account *acc = (struct Account *)accounts;
	for (int i = 0; i < numUsers; i++) {
		//offset = number of bytes to from start to byte of Status of each row
		offset += strlen((acc + i)->UserID) + strlen((acc + i)->Password) + 2;
		if (strcmp(username, (acc + i)->UserID) == 0) {
			FILE *fptr;
			fptr = fopen(file, "r+b");
			//set the stream pointer offset bytes from the start.
			fseek(fptr, offset, SEEK_SET);
			//set the Status byte to 1
			fputs("1", fptr);
			fclose(fptr);
			break;
		}
		//status is always have space is 1
		//after that is space of last location of row 
		//and first location of the next row
		offset += 3;
	}
}

void listAccount(char *accounts, int *numUser) {
	struct Account *acc = (struct Account *)accounts;
	char fusername[MSG_SIZE];
	char fpassword[MSG_SIZE];
	int status;
	int counter = 0;
	FILE *fptr;
	fptr = fopen(file, "r");
	fseek(fptr, 0, SEEK_SET);
	while (!feof(fptr)) {
		fscanf(fptr, "%s", fusername);
		fusername[strlen(fusername)] = 0;
		strcpy((acc + counter)->UserID, fusername);
		fscanf(fptr, "%s", fpassword);
		fpassword[strlen(fpassword)] = 0;
		strcpy((acc + counter)->Password, fpassword);
		fscanf(fptr, "%d", &status);
		(acc + counter)->Status = status;
		(acc + counter)->passFail = 0;
		counter++;
	}
	*numUser = counter;
}

bool checkLogedIn(char *username, list<const char*> session) {
	for (list<const char *>::iterator it = session.begin(); it != session.end(); ++it) {
		if (strcmp(username, *it) == 0) {
			return true;
		}
	}
	return false;
}

void incPassFail(char *username, char *accounts) {
	struct Account *acc = (struct Account *)accounts;
	for (int i = 0; i < numUsers; i++) {
		if (strcmp(username, (acc + i)->UserID) == 0)
			(acc + i)->passFail++;
	}
}

int numPassFail(char *username, char *accounts) {
	struct Account *acc = (struct Account *)accounts;
	for (int i = 0; i < numUsers; i++) {
		if (strcmp(username, (acc + i)->UserID) == 0)
			return (acc + i)->passFail;
	}
}