#include <iostream>
#include <process.h>
#include <string>
#include <WinSock2.h>
#include <algorithm>

#define DEFAULT_PORT 20001
#define DEFAULT_BUF 256

using namespace std;

enum IOTYPE { IDALLOC, RECV, SEND };

#pragma pack(push, 1)   
struct chatPacket {
	//char len;
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

struct PER_IO_DATA {
	OVERLAPPED overlapped;
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
	//HANDLE hCompletionPort;
	SOCKET hServSock;
	SOCKADDR_IN servAddr;
	PER_HANDLE_DATA * PerHandleData;
	PER_IO_DATA * PerIoData;
	CompletionPortMember* cpm = new CompletionPortMember;

	//int clientNum = 0;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		errorHandler("WSAStartup error");
	}

	cpm->ComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);

	for (int i = 0; i < (systemInfo.dwNumberOfProcessors + 5); i++)
		_beginthreadex(NULL, 0, CompletionThread, (LPVOID)cpm, 0, NULL);

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

	DWORD RecvBytes;
	DWORD Flags;
	int clientNum = 0;
	while (1) {

		SOCKET hClntSock;
		SOCKADDR_IN clntAddr;
		int clntAddrSz = sizeof(clntAddr);


		hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &clntAddrSz);
		/*if (hClntSock == SOCKET_ERROR) {
			cout << "accept error" << endl;
			continue;
		}*/
		u_long mode = 1;
		ioctlsocket(hClntSock, FIONBIO, &mode);
		/*if (WSAGetLastError() != WSAEWOULDBLOCK)
			errorHandler("accept error");*/
		PerHandleData = new PER_HANDLE_DATA;
		PerHandleData->hClntSock = hClntSock;
		memcpy(&PerHandleData->clntAddr, &clntAddr, clntAddrSz);
		cpm->clients[clientNum++] = PerHandleData;

		CreateIoCompletionPort((HANDLE)hClntSock, cpm->ComPort, (DWORD)PerHandleData, 0);

		PerIoData = new PER_IO_DATA;
		memset(&PerIoData->overlapped, 0, sizeof(OVERLAPPED));
		PerIoData->wsaRecvBuf.len = 0;
		PerIoData->wsaRecvBuf.buf = PerIoData->recvBuffer;
		PerIoData->operationType = IOTYPE::IDALLOC;

		Flags = 0;

		WSARecv(hClntSock, &PerIoData->wsaRecvBuf, 1,
			&RecvBytes, &Flags, &PerIoData->overlapped, NULL);
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

	int recvBytes;
	chatPacket packet;


	DWORD RecvBytes = 0;
	while (1) {
		GetQueuedCompletionStatus(CompletionPort, &BytesTransferred, 
			(LPDWORD)&PerHandleData, (LPOVERLAPPED*)&PerIoData, INFINITE);
		
		/*if (BytesTransferred == 0) {
			cout << "client 종료" << endl;
			closesocket(PerHandleData->hClntSock);
			delete PerHandleData;
			delete PerIoData;
		}*/
		recvBytes = 0;

		switch (PerIoData->operationType)
		{
		case IOTYPE::IDALLOC:
			while (true) {
				recvBytes += recv(PerHandleData->hClntSock, PerIoData->wsaRecvBuf.buf
					+ recvBytes, DEFAULT_BUF, 0);
				if (WSAGetLastError() == WSAEWOULDBLOCK) {

					copy(PerIoData->recvBuffer, PerIoData->recvBuffer + recvBytes,(char*) &packet);
					//memcpy(&packet, PerIoData->recvBuffer, DEFAULT_BUF);
					packet.message[recvBytes] = '\0';
					cout << packet.message << endl;
					packet.id = cpm->clients[PerHandleData->id]->id;

					memset(&PerIoData->overlapped, 0, sizeof(OVERLAPPED));
					PerIoData->wsaSendBuf.len = recvBytes;
					copy(&packet, &packet + recvBytes, (chatPacket* )PerIoData->sendBuffer);
					//memcpy(PerIoData->sendBuffer, &packet, recvBytes);
					PerIoData->wsaSendBuf.buf = PerIoData->sendBuffer;

					WSASend(PerHandleData->hClntSock, &(PerIoData->wsaSendBuf), 1, NULL, 0, &PerIoData->overlapped, NULL);
					

					break;
				}
			}
			break;
		case IOTYPE::RECV:

			break;
		case IOTYPE::SEND:

			break;
		default:
			break;
		}

		memset(&PerIoData->overlapped, 0, sizeof(OVERLAPPED));
		PerIoData->wsaRecvBuf.len = 0;
		PerIoData->wsaRecvBuf.buf = NULL;
		PerIoData->operationType = IOTYPE::IDALLOC;

		Flags = 0;

		WSARecv(PerHandleData->hClntSock, &PerIoData->wsaRecvBuf, 1,
			&RecvBytes, &Flags, &PerIoData->overlapped, NULL);
		

	}

}

void errorHandler(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
