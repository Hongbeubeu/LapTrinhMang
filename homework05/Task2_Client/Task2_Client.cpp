// Task2_Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdlib.h>
#include <cstdint>
#include <stdio.h>
#include <conio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

#define MESSAGE_SIZE 103
#define PAYLOAD_SIZE 100
#define FILENAME_SIZE 50

#pragma comment(lib,"ws2_32.lib")
using namespace std;

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
	//initiate parameter
	char fileName[FILENAME_SIZE];
	int ret, k;
	char opcode[2], length[3], tempk[4];
	char message[MESSAGE_SIZE],payload[PAYLOAD_SIZE];
	char len[MESSAGE_SIZE], result[MESSAGE_SIZE], receive[MESSAGE_SIZE];
	
	do
	{
		//input
		printf("\nFile Name: ");
		gets_s(fileName, FILENAME_SIZE);
		if (strcmp(fileName, "") == 0)
			break;
		FILE *f;
		f = fopen(fileName, "r");
		if (f == NULL) {
			printf("\nNot found file %s", fileName);
			continue;
		}
		printf("\npress 0 to encrypt, 1 to decrypt:	");
		gets_s(opcode, 2);
		strcpy(message, opcode);
		printf("\nkey k = ");
		gets_s(tempk, 4);
		k = atoi(tempk);
		ret = strlen(tempk);
		itoa(ret, length, 10);
		if (strlen(length) == 1) {
			char temp[3];
			strcpy(temp, length);
			strcpy(length, "0");
			strcat(length, temp);
		}
		strcat(message, length);
		itoa(k, payload, 10);
		strcat(message, payload);
		ret = strlen(message);
		message[ret] = 0;
		int num, len;
		//send key to server
		num = send(client, message, sizeof(message), 0);
		int i = 0;
		//send file to server
		while (!feof(f)) {
			if (i == PAYLOAD_SIZE-1) {
				payload[i] = 0;
				strcpy(message, "2");
				int payloadLength = strlen(payload);
				itoa(payloadLength, length, 10);
				strcat(message, length);
				strcat(message, payload);
				message[strlen(message)] = 0;
				num = send(client, message, sizeof(message), 0);
				i = 0;
			}
			char character;
			character = fgetc(f);
			payload[i] = character;	
			i++;
		}
		payload[i-1] = 0;
		strcpy(message, "2");
		int payloadLength = strlen(payload);
		itoa(payloadLength, length, 10);
		if (payloadLength < 10) {
			char temp[3];
			strcpy(temp, length);
			strcpy(length, "0");
			strcat(length, temp);
		}
		strcat(message, length);
		strcat(message, payload);
		message[payloadLength + 4] = 0;
		num = send(client, message, sizeof(message), 0);
		strcpy(message, "200");
		//send message to notify send file success
		num = send(client, message, sizeof(message), 0);
		fclose(f);
		//save file server sent back
		FILE *fout;
		strcat(fileName, ".enc");
		fout = fopen(fileName, "w+,ccs=UTF-8");
		do {
			ret = recv(client, message, MESSAGE_SIZE, 0);
			message[strlen(message)] = 0;
			length[0] = message[1];
			length[1] = message[2];
			length[2] = 0;
			len = atoi(length);
			if (len == 0)
				break;
			int i = 0;
			for (i = 0; i < len; i++)
				fprintf(fout, "%c", message[i+3]);
		} while (len != 0);
		fclose(fout);
		printf("Received file\n");
	}while (1);

	return 0;
}