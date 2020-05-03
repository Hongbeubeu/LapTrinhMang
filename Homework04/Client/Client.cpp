
#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#define BUFF_SIZE 2048
#define MSG_SIZE 50

#pragma comment(lib,"ws2_32.lib")

struct Message
{
	int type;
	char username[MSG_SIZE];
	char password[MSG_SIZE];
};

void menu();

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
	char len[BUFF_SIZE], result[BUFF_SIZE], receive[BUFF_SIZE], sizeMsg[BUFF_SIZE], iusername[MSG_SIZE], ipassword[MSG_SIZE];

	struct Message *messagePtr;
	messagePtr = (struct Message*)malloc(sizeof(struct Message));
	do
	{
		menu();
		int choose;
		fflush(stdin);
		scanf("%d", &choose);
		switch (choose)
		{
			case 1: {
				fflush(stdin);
				printf("\nusername: ");
				scanf("%s", iusername);
				strcpy(messagePtr->username, iusername);
				fflush(stdin);
				printf("\npassword: ");
				scanf("%s", ipassword);
				strcpy(messagePtr->password, ipassword);
				messagePtr->type = 1;
				ret = sizeof(*messagePtr);
				send(client, itoa(ret, sizeMsg, 10), sizeof(ret), 0);
				send(client, (char*)(messagePtr), sizeof(*messagePtr), 0);
				break;
			}
			case 2: {
				messagePtr->type = 2;
				ret = sizeof(*messagePtr);
				send(client, itoa(ret, sizeMsg, 10), sizeof(ret), 0);
				send(client, (char*)(messagePtr), sizeof(*messagePtr), 0);
				break;
			}
			case 3: {
				messagePtr->type = 3;
				ret = sizeof(*messagePtr);
				send(client, itoa(ret, sizeMsg, 10), sizeof(ret), 0);
				send(client, (char*)(messagePtr), sizeof(*messagePtr), 0);
				closesocket(client);
				break;
			}
		}

		//Get length of message from server
		ret = recv(client, len, BUFF_SIZE, 0);
		int length = atoi(len);
		result[0] = 0;
		do {
			//Receive message from server
			ret = recv(client, receive, BUFF_SIZE, 0);
			if (strlen(receive) > 0) {
				receive[ret] = 0;
				strcat(result, receive);
				length -= ret;
			}
		} while (length > 0);//get message until number of byte of message return zero
		printf("%s", result);
	} while (true);

	return 0;
}

void menu() {
	printf("\n1. Login\n");
	printf("2. Logout\n");
	printf("3. Exit\n");
	printf("Press key 1 to Login, 2 to Logout, 3 to Exit\n");
}