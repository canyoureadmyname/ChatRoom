// socketChatRoom.cpp : 
//

#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include<winsock2.h>
#include<iostream>
#include<thread>
#include<string>
#define DEFAULT_BUFLEN 512

WSADATA wsaData;
void RecvThread();
void SendThread();
SOCKET ClientSocket;
bool isLeft;

int main() {
	std::cout << "==LonelyLonelyCold_ChatRoom_Client==" << std::endl;
	int iResult;
	isLeft = false;
	WORD version = MAKEWORD(2, 2); //version 2.2
	iResult = WSAStartup(version, &wsaData);//init WinSock,return 0 if success
	if (iResult != 0) {
		std::cout << "WSAStartup failed: " << iResult << std::endl;
		return 1;
	}
	//set server ip & port which want to connect with
	std::string Server_ip;
	int Server_port;
	std::cout << "server ip: ";
	std::cin >> Server_ip;
	const char* c_server_ip = Server_ip.c_str();//convet string to const char*
	std::cout << "port: ";
	std::cin >> Server_port;
	// Setup the server information
	SOCKADDR_IN addr;
	addr.sin_addr.s_addr = inet_addr(c_server_ip);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(Server_port);

	while (1) {
		ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (ClientSocket == INVALID_SOCKET) {
			std::cout << "Socket error:" << WSAGetLastError() << std::endl;
			return 1;
		}
		if (connect(ClientSocket, (SOCKADDR*)&addr, sizeof(addr)) != 0) {//connect
			std::cout << "connection error: " << WSAGetLastError() << std::endl;
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

		std::cout << "Connection is successful." << std::endl << "Start Chatting..." << std::endl;

		std::thread send_thread(SendThread);
		std::thread recv_thread(RecvThread);

		send_thread.join();
		recv_thread.join();

		closesocket(ClientSocket);
		std::cout << "exit!!" << std::endl;
		char re;
		bool endpoint;
		std::cout << "Do you want to Reconnect ? (Y/N)" << std::endl;
		while (1) {//confirm input is right
			std::cin >> re;
			if (re == 'Y' || re == 'y' || re == 'N' || re == 'n')
				break;
			else {
				std::cout << "Invalid input . Please retry." << std::endl;
			}
		}
		switch (re){
			case 'Y' :
			case 'y' :
				endpoint = false;
				break;
			case 'N' :
			case 'n' :
				endpoint = true;
				break;
		}
		std::cout << std::endl;
		if (endpoint)//whether to close program
			break;
	}
	WSACleanup();
	std::cout << "Close." << std::endl;
	system("pause");
	return 0;
}

void SendThread() {
	int iResult;
	char sendbuf[DEFAULT_BUFLEN];
	std::cin.getline(sendbuf, 1);//eliminate previous \n
	while (1) {
		std::cin.getline(sendbuf, DEFAULT_BUFLEN);
		// Send the buffer
		iResult = send(ClientSocket, sendbuf, (int)strlen(sendbuf), 0);
		if (sendbuf[0] == '@') { return; }//exit
		if (iResult == SOCKET_ERROR) {
			int ERRcode = WSAGetLastError();
			if (ERRcode == WSAENOTCONN) { return; }//when disconnect
			else {
				std::cout << "send failed: " << ERRcode << std::endl;
				closesocket(ClientSocket);
				return;
			}
		}
	}
}

void RecvThread() {
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN];
	// Receive data until the server closes the connection
	do {
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			if (recvbuf[0] == '@') return;
			for (int i = 0; i < iResult; i++) {
				std::cout << recvbuf[i];
			}
			std::cout << std::endl;
		}
		else if (iResult == 0)
			std::cout << "Connection closed" << std::endl;
		else
			std::cout << "recv failed: " << WSAGetLastError() << std::endl;
	} while (iResult > 0);
	return;
}