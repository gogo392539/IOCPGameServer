#include <iostream>
#include <process.h>
#include <string>
#include <WinSock2.h>
#include <algorithm>
#include <mutex>

#define DEFAULT_PORT 20001
#define DEFAULT_BUF 257
#define FLAG_BUF 2

//using std::cout;
//using std::endl;
using namespace std;

enum IOTYPE { IDALLOC, RECV, SEND };

#pragma pack(push, 1)   
struct chatPacket {
	char flag;
	char len;
	char id;
	char message[256];
};
#pragma pack(pop)

struct PER_HANDLE_DATA {
	SOCKET hClntSock;
	SOCKADDR_IN clntAddr;
	int id;
	//char nickname[32];
};

struct PER_IO_DATA {
	OVERLAPPED overlapped;
	chatPacket recvPacket;
	chatPacket sendPacket;
	WSABUF wsaRecvBuf;
	WSABUF wsaSendBuf;
	IOTYPE operationType;
};

struct CompletionPortMember {
	HANDLE ComPort;
	PER_HANDLE_DATA* clients[5];
};			

#pragma pack(push, 1)
struct clientInfo {
	int id;
	char nickname[32];
};							// client들의 id와 nickname을 저장하는 구조체
#pragma pack(pop)

clientInfo clntInfo[5];

unsigned int __stdcall CompletionThread(HANDLE CompPortMem);
void errorHandler(char* message);
int recvn(SOCKET socket, char* buf, int len, int flag);

int main(void) {
	WSAData wsaData;
	//HANDLE hCompletionPort;
	SOCKET hServSock;
	SOCKADDR_IN servAddr;
	PER_HANDLE_DATA * PerHandleData;
	PER_IO_DATA * PerIoData;
	CompletionPortMember* cpm = new CompletionPortMember;

	for (int i = 0; i < 5; i++) {
		clntInfo[i].id = -1;
		memset(clntInfo[i].nickname, 0, sizeof(clntInfo[i].nickname));
	}

	int nZero;

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

		/* 
		setsockopt() 함수를 통해 send와 recv의 커널 단 버퍼의 크기를 0으로 만들고 있다.
		IOCP가 Overlapped IO에 대한 결과를 통보 받는 메커니즘이기 때문에 커널단 버퍼를 사용하지 않고
		직접 제공된 버퍼를 사용한다.
		*/
		nZero = 0;
		if (SOCKET_ERROR == setsockopt(hClntSock, SOL_SOCKET, SO_RCVBUF, (const char*)&nZero, sizeof(int))) {
			errorHandler("setsockopt error");
		}

		nZero = 0;
		if (SOCKET_ERROR == setsockopt(hClntSock, SOL_SOCKET, SO_SNDBUF, (const char*)&nZero, sizeof(int))) {
			errorHandler("setsockopt error");
		}
		
		u_long mode = 1;
		ioctlsocket(hClntSock, FIONBIO, &mode);
		
		PerHandleData = new PER_HANDLE_DATA;
		PerHandleData->hClntSock = hClntSock;
		memcpy(&PerHandleData->clntAddr, &clntAddr, clntAddrSz);
		PerHandleData->id = clientNum;
		cpm->clients[clientNum++] = PerHandleData;		// client socket, socketaddr, client id, client nickname을 저장

		CreateIoCompletionPort((HANDLE)hClntSock, cpm->ComPort, (DWORD)PerHandleData, 0);

		PerIoData = new PER_IO_DATA;
		memset(&PerIoData->overlapped, 0, sizeof(OVERLAPPED));
		PerIoData->wsaRecvBuf.len = 0;
		PerIoData->wsaRecvBuf.buf = (char*)&(PerIoData->recvPacket);
		//PerIoData->operationType = IOTYPE::IDALLOC;

		Flags = 0;

		WSARecv(hClntSock, &PerIoData->wsaRecvBuf, 1,
			&RecvBytes, &Flags, &PerIoData->overlapped, NULL);
	}

	WSACleanup();
	return 0;
}

unsigned int __stdcall CompletionThread(HANDLE CompPortMem) {
	std::mutex mutex;
	CompletionPortMember* cpm = (CompletionPortMember*)CompPortMem;
	HANDLE CompletionPort = (HANDLE)cpm->ComPort;
	DWORD BytesTransferred;
	DWORD Flags;
	PER_HANDLE_DATA* PerHandleData;
	PER_IO_DATA* PerIoData;

	int recvBytes;
	int messageLen, id, flag;
	char tempNick[32];
	chatPacket packet;


	DWORD RecvBytes = 0;
	while (1) {
		recvBytes = 0;
		GetQueuedCompletionStatus(CompletionPort, &BytesTransferred, 
			(LPDWORD)&PerHandleData, (LPOVERLAPPED*)&PerIoData, INFINITE);
		
		recvBytes = recvn(PerHandleData->hClntSock, PerIoData->wsaRecvBuf.buf, 3, 0);
		flag = (int)PerIoData->recvPacket.flag;
		messageLen = (int)PerIoData->recvPacket.len;
		PerIoData->recvPacket.id = PerHandleData->id;

		switch (flag)
		{
		case 0:				// client chatting
			
			
			break;
		case 1:				// client enterance
			RecvBytes = recvn(PerHandleData->hClntSock, PerIoData->wsaRecvBuf.buf + recvBytes, messageLen, 0);

			mutex.lock();
			clntInfo[PerHandleData->id].id = PerHandleData->id;
			std::copy(PerIoData->recvPacket.message, PerIoData->recvPacket.message + messageLen, clntInfo[PerHandleData->id].nickname);
			clntInfo[PerHandleData->id].nickname[messageLen] = '\0';
			mutex.unlock();

			cout << clntInfo[PerHandleData->id].nickname << " User에게 ID " << clntInfo[PerHandleData->id].id << " 할당" << endl;
			//cout << clntInfo[PerHandleData->id].nickname  << clntInfo[PerHandleData->id].id  << endl;


			PerIoData->sendPacket.flag = flag;
			PerIoData->sendPacket.len = 0;
			PerIoData->sendPacket.id = PerHandleData->id;
			send(PerHandleData->hClntSock, (char*)&PerIoData->sendPacket, 3, 0);

			std::copy((char*)&clntInfo, (char*)&clntInfo + sizeof(clntInfo), PerIoData->sendPacket.message);

			/*for (int i = 0; i < 5; i++) {
				if (clntInfo[i].id != -1) {
					send(cpm->clients[i]->hClntSock, (char*)&PerIoData->sendPacket, 3 + sizeof(clntInfo), 0);
				}
			}*/
			break;
		default:
			break;
		}

		memset(&PerIoData->overlapped, 0, sizeof(OVERLAPPED));
		memset(&PerIoData->recvPacket, 0, sizeof(PerIoData->recvPacket));
		PerIoData->wsaRecvBuf.len = 0;
		PerIoData->wsaRecvBuf.buf = (char*)&(PerIoData->recvPacket);
		//PerIoData->operationType = IOTYPE::IDALLOC;

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

int recvn(SOCKET socket, char* buf, int len, int flag) {
	int nleft;
	int nrecv;

	nleft = len;
	while (nleft > 0) {
		nrecv = recv(socket, buf, nleft, flag);

		if (nrecv < 0) {
			return (SOCKET_ERROR);        /* error */
		}
		else if (nrecv == 0) {
			break;						  /*EOF*/
		}

		nleft -= nrecv;
		buf += nrecv;
	}
	return (len - nleft);
}
