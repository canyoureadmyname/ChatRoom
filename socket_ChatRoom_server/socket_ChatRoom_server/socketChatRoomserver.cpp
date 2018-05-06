// socketChatRoomserver.cpp : 
//

#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<winsock2.h>
#include<iostream>
#include<thread>
#include<sstream>
#include<string>
#define DEFAULT_BUFLEN 512
#define MAX_CONNECTION 100
WSADATA wsaData;
bool isConnected[MAX_CONNECTION];//whether client socket connect
bool needJoin[MAX_CONNECTION];//whether thread need join
void RecvAndSendThread(SOCKET* ClientSocket, int index);

int main() {

	std::cout << "==LonelyLonelyCold_ChatRoom_Server==" << std::endl;
	int iResult;
	WORD version = MAKEWORD(2, 2); //version 2.2
	iResult = WSAStartup(version, &wsaData);//init WinSock,return 0 if success
	if (iResult != 0) {
		std::cout << "WSAStartup failed: " << iResult << std::endl;
		return 1;
	}
	SOCKET ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ServerSocket == INVALID_SOCKET) {
		std::cout << "Socket error:" << WSAGetLastError() << std::endl;
		return 1;
	}
	// Setup the TCP listening socket
	int listen_port;
	std::cout << "Listen port : ";
	std::cin >> listen_port;
	std::cout << "Start listening..." << std::endl;
	SOCKADDR_IN addr;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_family = AF_INET;
	addr.sin_port = htons(listen_port);
	iResult = bind(ServerSocket, (SOCKADDR*)&addr, sizeof(addr));
	if (iResult == SOCKET_ERROR) {
		std::cout << "bind failed with error:" << WSAGetLastError() << std::endl;
		closesocket(ServerSocket);
		WSACleanup();
		return 1;
	}
	if (listen(ServerSocket, SOMAXCONN) == SOCKET_ERROR) {
		std::cout << "Listen failed with error:" << WSAGetLastError() << std::endl;
		closesocket(ServerSocket);
		WSACleanup();
		return 1;
	}
	SOCKET ClientSocket[MAX_CONNECTION];
	SOCKADDR_IN ClientAddr[MAX_CONNECTION];
	std::thread recv_send_thread[MAX_CONNECTION];
	int ClientAddr_len = sizeof(ClientAddr);
	int index;
	for (int i = 0; i < MAX_CONNECTION; i++) { //init
		isConnected[i] = false; 
		needJoin[i] = false;
	}
	while (1) {
		index = -1;
		while (1) {//until find empty socket space
			for (int i = 0; i < MAX_CONNECTION; i++) {//find no connection
				if (!isConnected[i]) {
					if (needJoin[i]) {//join no use thread
						recv_send_thread[i].join();
						closesocket(ClientSocket[i]);
					}
					index = i;
					break;
				}
			}
			if (index != -1) break;
		}
		// Accept the client socket
		ClientSocket[index] = accept(ServerSocket, (SOCKADDR*)&ClientAddr[index], &ClientAddr_len);
		if (ClientSocket[index] == INVALID_SOCKET) {
			std::cout << "accept failed:" << WSAGetLastError() << std::endl;
			closesocket(ServerSocket);
			WSACleanup();
			return 1;
		}
		else {
			std::cout << "Got connection from " << inet_ntoa(ClientAddr[index].sin_addr) << std::endl;
			isConnected[index] = true;
			needJoin[index] = false;
			std::string CodeName_sendbuf = "Your code name is ";//send client its code name
			std::stringstream ss;//convert int to string
			ss << index;
			CodeName_sendbuf += ss.str();
			iResult = send(ClientSocket[index], CodeName_sendbuf.c_str(), (int)strlen(CodeName_sendbuf.c_str()), 0);
			if (iResult == SOCKET_ERROR) {
				std::cout << "send failed: " << WSAGetLastError() << std::endl;
				closesocket(ClientSocket[index]);
				return 1;
			}
			else {
				std::cout << "assign code name : "<< index << std::endl;
			}
			recv_send_thread[index] = std::thread(RecvAndSendThread, ClientSocket, index);
		}
	}
	WSACleanup();
	system("pause");
	return 0;
}

void RecvAndSendThread(SOCKET* ClientSocket, int index) {//receive from ClientSocket[index],broadcast to every connect client
	char recvbuf[DEFAULT_BUFLEN];
	int iSendResult;
	int recvbuflen = DEFAULT_BUFLEN;
	int iResult;
	std::stringstream ss;
	std::string sendbuf ="";
	// Receive until the peer shuts down the connection
	do {

		iResult = recv(ClientSocket[index], recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			if (recvbuf[0] == '@') {//client want leave
				iSendResult = send(ClientSocket[index], recvbuf, iResult, 0);
				if (iSendResult == SOCKET_ERROR) {
					std::cout << "code name: " << index << "send failed: " << WSAGetLastError() << std::endl;
					closesocket(ClientSocket[index]);
					return;
				}
				std::cout << "code name: " << index << " just leave the room." << std::endl;
				isConnected[index] = false;
				needJoin[index] = true;
				return;
			}
			std::cout << "[" << index << "] : ";
			for (int i = 0; i<iResult; i++) {
				std::cout << recvbuf[i];
				sendbuf += recvbuf[i];
			}
			std::cout << std::endl;
			// Echo the buffer back to the sender
			for (int i = 0; i < MAX_CONNECTION; i++) {
				if (isConnected[i] && (i!=index)) {
					ss << '[' << index << "] : ";
					sendbuf = ss.str() + sendbuf;
					iSendResult = send(ClientSocket[i], sendbuf.c_str(), (int)strlen(sendbuf.c_str()), 0);
					if (iSendResult == SOCKET_ERROR) {
						std::cout << "code name: " << i << "send failed: " << WSAGetLastError() << std::endl;
						closesocket(ClientSocket[i]);
						return;
					}
				}
			}
		}
		else if (iResult == 0)
			std::cout <<"code name: "<<index<< " just leave the room." << std::endl;
		else {
			std::cout <<"code name: " <<index<< "recv failed: " << WSAGetLastError() << std::endl;
			closesocket(ClientSocket[index]);
			return;
		}
	} while (iResult > 0);
	isConnected[index] = false;
	needJoin[index] = true;
	return;
}
