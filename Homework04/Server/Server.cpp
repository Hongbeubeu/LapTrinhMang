// Server.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>	
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <process.h>
#include <string>
#include <list>
#include <iostream>

#define SERVER_ADDRF "127.0.0.1"
#define BUFF_SIZE 2048
#define LOGIN 1
#define LOGOUT 2
#define SIZE 10
#define MSG_SIZE 50
#pragma comment(lib,"ws2_32.lib")

using namespace std;

const char file[] = "account.txt";

struct Message {
	int type;
	char username[MSG_SIZE];
	char password[MSG_SIZE];
};

struct Account {
	char UserID[MSG_SIZE];
	char Password[MSG_SIZE];
	int Status;
};

//accounts contain account read in file acounts.txt
struct Account accounts[SIZE];

//session contain account is loged in
list<const char*> session;
int numUsers;
unsigned int __stdcall  ServClient(void *data);

bool checkUsername(char *username, char *accounts);
bool isActived(char *username, char *accounts);
bool checkPassword(char *username, char *password, char *accounts);
void lockAccount(char *username, char *accounts);
void listAccount(char *accounts, int *numUser);
bool checkLogedIn(char *username, list<const char*> session);


int main(int argc, char* argv[])
{
	if (argc != 2) {
		printf("Not found parameter or to much parameter");
		return 0;
	}

	int	SERVER_PORT = atoi(argv[1]);

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
	listAccount((char *)accounts, &numUsers);

	//Step 5: Communicate with client
	sockaddr_in clientAddr;

	//initiate parameter
	char buff[BUFF_SIZE], len[BUFF_SIZE], receive[BUFF_SIZE];
	int ret, clientAddrLen = sizeof(clientAddr), length;
	SOCKET connSock;
	
	//Handle message
	while (1){
		connSock = accept(listenSock, (sockaddr *)& clientAddr, &clientAddrLen);
		_beginthreadex(0, 0, ServClient, (void*)&connSock, 0, NULL);
	}

	closesocket(listenSock);

	//Terminate Winsock
	WSACleanup();

	return 0;
}

unsigned int __stdcall ServClient(void *data)
{
	SOCKET	*client = (SOCKET*)data;
	SOCKET Client = *client;
	printf("\nClient: [%d] : Connected\n", GetCurrentThreadId());
	//initiate parameter
	char buff[BUFF_SIZE], len[BUFF_SIZE], receive[BUFF_SIZE], replyMessage[BUFF_SIZE], sizeMsg[BUFF_SIZE];
	int ret, length;

	int incorrectCounter = 0;
	bool isLogin = false;

	struct Message *receiveMessage;

	while (1) {
		ret = recv(Client, len, BUFF_SIZE, 0);
		if (ret == SOCKET_ERROR) {
			printf("\nClient: [%d] : Disconnected\n", GetCurrentThreadId());
			return 0;
		}
		length = atoi(len);
		buff[0] = 0;
		do {
			//Receive message from client
			ret = recv(Client, receive, BUFF_SIZE, 0);
			if (ret == SOCKET_ERROR) {
				printf("\nClient: [%d] : Disconnected\n", GetCurrentThreadId());
				return 0;
			}
			if (strlen(receive) > 0) {
				receive[ret] = 0;
				strcat(buff, receive);
				length -= ret;
			}
		} while (length > 0);//get message until number of byte of message return zero

		buff[strlen(buff)] = 0;
		strcpy(receive, buff);
		receiveMessage = (struct Message*)receive;
		int typeMessage = receiveMessage->type;
		
		//if socket is closed, message don't send to client
		bool isSocketClosed = false;
		switch (typeMessage) {
			case LOGIN: {
				if (checkUsername(receiveMessage->username, (char*)accounts)) {
					if (!isActived(receiveMessage->username, (char*)accounts)) {
						printf("Server: Account is locked\n");
						strcpy(replyMessage, "Server: Account is locked\n");
						break;
					}
					else if (checkLogedIn(receiveMessage->username, session)) {
						printf("Server: This Account is loged in another client\n");
						strcpy(replyMessage, "Server: This Account is loged in another client\n");
						break;
					}
					else if (checkPassword(receiveMessage->username, receiveMessage->password, (char*)accounts)) {
						if (!isLogin) {
							printf("Server: Login success\n");
							strcpy(replyMessage, "Server: Login success\n");
							isLogin = true;
							session.push_back(receiveMessage->username);
							printf("Hello %s\n", receiveMessage->username);
							break;
						}
						else {
							printf("Server: Client is already login\n");
							strcpy(replyMessage, "Server: Client is already login\n");
							break;
						}
					}
					else {
						incorrectCounter++;
						if (incorrectCounter >= 3) {
							printf("Server:This account is locked because try to access into server with incorrect password three times\n");
							strcpy(replyMessage, "Server: This account is locked because try to access into server with incorrect password three times\n");
							lockAccount(receiveMessage->username, (char *)accounts);
							listAccount((char *)accounts, &numUsers);
							break;
						}
						else {
							printf("Server: Incorrect password\n");
							strcpy(replyMessage, "Server: Incorrect password\n");
							break;
						}
					}
				}
				else {
					printf("Server: Incorrect username\n");
					strcpy(replyMessage, "Server: Incorrect username\n");
					break;
				}
			}
			case LOGOUT: {
				if (isLogin) {
					printf("Server: Logout!\n");
					strcpy(replyMessage, "Server: Logout!\n");
					isLogin = false;
					session.remove(receiveMessage->username);
					break;
				}
				else {
					printf("Server: You haven't login\n");
					strcpy(replyMessage, "Server: You haven't login\n");
					break;
				}
			}
			default: {
				isSocketClosed = true;
				shutdown(Client, SD_BOTH);
				printf("\nClient: [%d] : Disconnected\n", GetCurrentThreadId());
				session.remove(receiveMessage->username);
				return 0;
			}
		}
		if (!isSocketClosed) {
			ret = strlen(replyMessage);
			//send length of message to client
			ret = send(Client, itoa(ret, sizeMsg, 10), ret, 0);
			//send message to client
			ret = send(Client, replyMessage, strlen(replyMessage), 0);
		}
	}
	return 0;
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
		if(strcmp(username, (acc + i)->UserID) == 0)
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
