#include <iostream>
#include <process.h>
#include <string>
#include <WinSock2.h>
#include <vector>

#define DEFAULT_PORT 20001
#define DEFAULT_BUF 512

using namespace std;

enum IOTYPE { ZERORECV, RECV, SEND, ID_SEND, NICK_RECV, JOIN_SEND };
/*
ID_SEND : client 접속시 id 전달
NICK_RECV : client가 보낸 nickname 수신
JOIN_SEND : 새로운 client가 접속하면 ID와 nickname을 모든 client에게 전달
*/

#pragma pack(push, 1)   
struct chatPacket {
	char len;
	char id;
	char message[256];
};
#pragma pack(pop)

struct PER_HANDLE_DATA {
	SOCKET hClntSock;
	SOCKADDR_IN clntAddr;
	int id;
	string nickname;
};

struct PER_IO_DATA : OVERLAPPED {
	char recvBuffer[DEFAULT_BUF];
	char sendBuffer[DEFAULT_BUF];
	WSABUF wsaRecvBuf;
	WSABUF wsaSendBuf;
	IOTYPE operationType;
};

struct CompletionPortMember {
	HANDLE ComPort;
	PER_HANDLE_DATA* clients[5];
};

unsigned int __stdcall CompletionThread(HANDLE CompPortMem);
void errorHandler(char* message);

int main(void) {
	WSAData wsaData;
	HANDLE hCompletionPort;
	SOCKET hServSock;
	SOCKADDR_IN servAddr;
	PER_HANDLE_DATA * PerHandleData;
	PER_IO_DATA * PerIoData;
	CompletionPortMember* cpm = new CompletionPortMember;

	//int clientNum = 0;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		errorHandler("WSAStartup error");
	}

	hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 5);

	for (int i = 0; i < 5; i++) {
		_beginthreadex(NULL, 0, CompletionThread, (LPVOID)cpm, 0, NULL);
	}

	hServSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = PF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(DEFAULT_PORT);

	if (::bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
		errorHandler("bind error");
	}

	if (listen(hServSock, 5) == SOCKET_ERROR) {
		errorHandler("listen error");
	}

	while (1) {

		SOCKET hClntSock;
		SOCKADDR_IN clntAddr;
		int clntAddrSz = sizeof(clntAddr);
		DWORD RecvBytes;
		DWORD Flags;
		int clientNum = 0;

		hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &clntAddrSz);
		if (hClntSock == SOCKET_ERROR) {
			cout << "accept error" << endl;
			continue;
		}

		PerHandleData = new PER_HANDLE_DATA;
		PerHandleData->hClntSock = hClntSock;
		memcpy(&PerHandleData->clntAddr, &clntAddr, clntAddrSz);
		cpm->clients[clientNum++] = PerHandleData;

		CreateIoCompletionPort((HANDLE)hClntSock, hCompletionPort, (DWORD)PerHandleData, 0);

		PerIoData = new PER_IO_DATA;
		memset((LPOVERLAPPED)&PerIoData, 0, sizeof(OVERLAPPED));
		PerIoData->wsaRecvBuf.len = 0;
		PerIoData->wsaRecvBuf.buf = PerIoData->recvBuffer;
		PerIoData->operationType = IOTYPE::ZERORECV;

		Flags = 0;

		WSARecv(hClntSock, &PerIoData->wsaRecvBuf, 1, &RecvBytes, &Flags, (LPOVERLAPPED)&PerIoData, NULL);
	}

	WSACleanup();
	return 0;
}

unsigned int __stdcall CompletionThread(HANDLE CompPortMem) {
	CompletionPortMember* cpm = (CompletionPortMember*)CompPortMem;
	HANDLE CompletionPort = (HANDLE)cpm->ComPort;
	DWORD BytesTransferred;
	DWORD Flags;
	PER_HANDLE_DATA* PerHandleData;
	PER_IO_DATA* PerIoData;

	while (1) {
		GetQueuedCompletionStatus(CompletionPort, &BytesTransferred, (LPDWORD)&PerHandleData, (LPOVERLAPPED*)&PerIoData, INFINITE);
		
		/*if (BytesTransferred == 0) {
			cout << "client 종료" << endl;
			closesocket(PerHandleData->hClntSock);
			delete PerHandleData;
			delete PerIoData;
		}*/

		switch (PerIoData->operationType)
		{
		case IOTYPE::ZERORECV:
			
			break;
		case IOTYPE::RECV:

			break;
		case IOTYPE::SEND:

			break;
		default:
			break;
		}




	}

}

void errorHandler(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}