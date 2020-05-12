// Task2_Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>	
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <process.h>
#include <iostream>
#include <string>
#include <direct.h>

#define SERVER_ADDRF "127.0.0.1"
#define MESSAGE_SIZE 103
#define PAYLOAD_SIZE 100
#define FILENAME_SIZE 50
#define ASCII_SIZE 256

#pragma comment(lib,"ws2_32.lib")

using namespace std;

unsigned int __stdcall  ServClient(void *data);
void encode(char *inputFileName, char *outputFileName, int k);
void decode(char *inputFileName, char *outputFileName, int k);
int main(int argc, char* argv[]){
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

	//Step 5: Communicate with client
	sockaddr_in clientAddr;

	//initiate parameter
	int clientAddrLen = sizeof(clientAddr);
	SOCKET connSock;
	
	//create folder to save file, which client sent to server
	_mkdir("./ClientFile");
	
	//Handle message
	while (1) {
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
	//initiate socket with client in thread
	SOCKET	*client = (SOCKET*)data;
	SOCKET Client = *client;

	//initiate parameter
	char message[MESSAGE_SIZE], length[3], payload[PAYLOAD_SIZE], opcode[2];
	int len, k;

	printf("\nClient: [%d] : Connected\n", GetCurrentThreadId());

	//locale utf8{ "en_us.UTF-8" };
	do {
		//receive message contain key
		int ret = recv(Client, message, MESSAGE_SIZE, 0);
		if (ret == SOCKET_ERROR) {
			printf("\nClient: [%d] : Disconnected\n", GetCurrentThreadId());
			return 0;
		}
		opcode[0] = message[0];
		opcode[1] = 0;
		length[0] = message[1];
		length[1] = message[2];
		length[2] = 0;
		len = atoi(length);
		int i = 0;
		for (i = 0; i < len; i++) {
			payload[i] = message[i + 3];
		}
		payload[i] = 0;
		k = atoi(payload);

		//initiate file to contain temp file
		_putenv("TMP=");
		char *inputFileName = NULL, *outputFileName = NULL;
		inputFileName = _tempnam("./ClientFile\\", "input");
		outputFileName = _tempnam("./ClientFile\\", "output");
		FILE *fin;
		fin = fopen(inputFileName, "w+b");
		message[strlen(message)] = 0;

		do {
			//receive message contain file
			ret = recv(Client, message, MESSAGE_SIZE, 0);
			if (ret == SOCKET_ERROR) {
				printf("\nClient: [%d] : Disconnected\n", GetCurrentThreadId());
				return 0;
			}
			message[strlen(message)] = 0;
			length[0] = message[1];
			length[1] = message[2];
			length[2] = 0;
			len = atoi(length);
			if (len == 0)
				break;
			int i = 0;
			for (i = 0; i < len; i++)
				payload[i] = message[i + 3];
			payload[i] = 0;
			fprintf(fin, "%s", payload);
		} while (len != 0);
		fclose(fin);
		if (opcode[0] == '0') {
			encode(inputFileName, outputFileName, k);
			printf("Client: [%d] : Encode success\n", GetCurrentThreadId());
		}
		else {
			decode(inputFileName, outputFileName, k);
			printf("Client: [%d] : Decode success\n", GetCurrentThreadId());
		}
		//write file receive to temp file
		FILE *f;
		f = fopen(outputFileName, "rb");
		int j = 0;
		//send file output to client
		while (!feof(f)) {
			if (j == PAYLOAD_SIZE - 1) {
				payload[j] = 0;
				strcpy(message, "2");
				int payloadLength = strlen(payload);
				itoa(payloadLength, length, 10);
				strcat(message, length);
				strcat(message, payload);
				message[strlen(message)] = 0;
				//send part of file out put to client
				ret = send(Client, message, sizeof(message), 0);
				j = 0;
			}
			char character;
			character = fgetc(f);
			payload[j] = character;
			j++;
		}
		payload[j - 1] = 0;
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
		ret = send(Client, message, sizeof(message), 0);
		strcpy(message, "200");
		//send message to notify send file success
		ret = send(Client, message, sizeof(message), 0);
		fclose(f);

		//remove temp file
		remove(inputFileName);
		remove(outputFileName);
		printf("Client: [%d] : Removed temp file\n", GetCurrentThreadId());
	} while (1);
	return 0;
}

//encode
void encode(char *inputFileName, char *outputFileName, int k) {
	FILE *fin, *fout;
	fin = fopen(inputFileName, "rb");
	fout = fopen(outputFileName, "w+b");
	int i = 0;
	
	unsigned int m, c;
	while (!feof(fin)) {
		m = fgetc(fin);
		if (m == EOF)	
			break;
		c = (m + k) % ASCII_SIZE;
		fprintf(fout, "%c", c);
	}
	fclose(fin);
	fclose(fout);
}

//decode
void decode(char *inputFileName, char *outputFileName, int k) {
	FILE *fin, *fout;
	fin = fopen(inputFileName, "rb");
	fout = fopen(outputFileName, "w+b");
	unsigned int m, c;
	while (!feof(fin)) {
		c = fgetc(fin);
		if (c == EOF)
			break;
		m = (c - k) % ASCII_SIZE;
		fprintf(fout, "%c", m);
	}
	fclose(fin);
	fclose(fout);
}
