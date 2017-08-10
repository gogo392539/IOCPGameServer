#include <iostream>
#include <algorithm>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>

#include "../../lib/source/IOlib.h"

#pragma comment(lib, "../../lib/debug_lib/IOlib_d.lib")

#define PORT 20003
#define LEN_ID_FLAGS_SIZE 3
#define NICK_MAX_LEN 32
#define ID_NOT_ALLOC -1
#define CLIENT_MAX 5
#define MESSAGE_FLAG 0
#define ID_ALLOC_FLAG 1
#define EVENT_MAX 2
#define HEADER_RECV_EVENT 0
#define MESSAGE_RECV_EVENT 1
#define BUF_SIZE 256

using namespace std;

void ErrorHandling(char* msg);

void RecvThread(SOCKET clntSock);
void SendThread(SOCKET clntSock);

//2017.08.07 손기문 맨 처음 id 할당받을때 이 구조체의 id로 받아옴
#pragma pack(push, 1)   
struct chatPacket {
	char len;
	char id;
	char flags;
	char message[BUF_SIZE];
};
#pragma pack(pop)

chatPacket recvPacket;
chatPacket sendPacket;
WSABUF recvBuf;
WSABUF sendBuf;
char myNick[NICK_MAX_LEN];
int myId = 0;
char clientNick[CLIENT_MAX][NICK_MAX_LEN];


int main(void) {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup Error!");

	SOCKET clntSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN servAddr;
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, "127.0.0.1", &servAddr.sin_addr);
	if (connect(clntSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("connect Error!");

	cout << "닉네임을 입력하세요 : ";
	cin >> myNick;
	int nickLen = strlen(myNick);
	sendPacket.id = ID_NOT_ALLOC;
	sendPacket.len = nickLen;
	sendPacket.flags = ID_ALLOC_FLAG;
	copy(myNick, myNick + nickLen, sendPacket.message);

	// 닉네임 보내기
	int flags = 0;
	if (sendn(clntSock, (char*)&sendPacket, sendPacket.len + LEN_ID_FLAGS_SIZE, flags)
		== SOCKET_ERROR)
		ErrorHandling("NICK send Error!");

	// 본인 아이디 받기
	if (recvn(clntSock, (char*)&recvPacket, 3, flags) == SOCKET_ERROR)
		ErrorHandling("IDLEN recv Error!");
	if (recvPacket.flags == ID_ALLOC_FLAG) {
		myId = recvPacket.id;
		copy(myNick, myNick + nickLen, clientNick[myId]);
	}

	// recv,  send 스레드 시작
	thread recvThread(RecvThread, clntSock);
	thread sendThread(SendThread, clntSock);

	recvThread.join();
	sendThread.join();
	WSACleanup();
	return 0;
}

void ErrorHandling(char* msg) {
	cout << msg << endl;
	exit(1);
}

void RecvThread(SOCKET clntSock) {
	WSAEVENT wsaEvent[EVENT_MAX];
	int eventNum = HEADER_RECV_EVENT;
	wsaEvent[HEADER_RECV_EVENT] = WSACreateEvent();
	wsaEvent[MESSAGE_RECV_EVENT] = WSACreateEvent();
	WSAOVERLAPPED overlapped;
	int recvBytes = 0;
	int headerRecvBytes = 0;
	int messageRecvBytes = 0;
	int flags = 0;
	int index = 0;
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = wsaEvent[eventNum];
	recvBuf.len = LEN_ID_FLAGS_SIZE;
	recvBuf.buf = (char*)&recvPacket;

	//헤더 recv
	if (WSARecv(clntSock, &recvBuf, 1, (DWORD*)&recvBytes, (DWORD *)&flags,
		&overlapped, NULL) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING)
			ErrorHandling("RecvThread recv1 Error");
	}
	while (true) {
		index = WSAWaitForMultipleEvents(EVENT_MAX, wsaEvent, false, WSA_INFINITE, false);
		WSAResetEvent(wsaEvent[index - WSA_WAIT_EVENT_0]);

		WSAGetOverlappedResult(clntSock, &overlapped, (DWORD*)&recvBytes, false, (DWORD*)&flags);
		if (recvBytes == 0) {
			ErrorHandling("RecvThread RecvHeader1 Error!");
		}

		switch (index - WSA_WAIT_EVENT_0) {
		case HEADER_RECV_EVENT:
			headerRecvBytes += recvBytes;
			if (headerRecvBytes == LEN_ID_FLAGS_SIZE) {
				headerRecvBytes = 0;

				memset(recvPacket.message, 0, sizeof(recvPacket.message));
				recvBuf.buf = recvPacket.message;
				recvBuf.len = recvPacket.len;
				memset(&overlapped, 0, sizeof(WSAOVERLAPPED));
				overlapped.hEvent = wsaEvent[MESSAGE_RECV_EVENT];

				cout << "header recv 1 : " << (int)recvPacket.flags << ", " << (int)recvPacket.id
					<< ", " << (int)recvPacket.len << endl;

				//메시지 recv
				if (WSARecv(clntSock, &recvBuf, 1, (DWORD*)&recvBytes, (DWORD *)&flags,
					&overlapped, NULL) == SOCKET_ERROR) {
					if (WSAGetLastError() != WSA_IO_PENDING)
						ErrorHandling("RecvThread RecvHeader2 Error");
				}
				continue;	// 컨티뉴 안쓸 수 있는지 고민해보기
			}
			//헤더 다 못받았을 경우 추가 recv
			else if (headerRecvBytes < LEN_ID_FLAGS_SIZE) {
				recvBuf.buf = ((char*)&recvPacket) + recvBytes;
				recvBuf.len = LEN_ID_FLAGS_SIZE - recvBytes;
				memset(&overlapped, 0, sizeof(WSAOVERLAPPED));
				overlapped.hEvent = wsaEvent[HEADER_RECV_EVENT];

				if (WSARecv(clntSock, &recvBuf, 1, (DWORD*)&recvBytes, (DWORD *)&flags,
					&overlapped, NULL) == SOCKET_ERROR) {
					if (WSAGetLastError() != WSA_IO_PENDING)
						ErrorHandling("RecvThread RecvMessage1 Error");
				}
				continue;	// 컨티뉴 안쓸 수 있는지 고민해보기
			}
			break;
		case MESSAGE_RECV_EVENT:
			messageRecvBytes += recvBytes;
			if (messageRecvBytes == recvBuf.len) {
				messageRecvBytes = 0;

				switch (recvPacket.flags) {
				case ID_ALLOC_FLAG:
					copy(recvPacket.message, recvPacket.message + recvPacket.len,
						clientNick[recvPacket.id]);

					cout << "header recv 2 : " << (int)recvPacket.flags << ", " << (int)recvPacket.id
						<< ", " << (int)recvPacket.len << ", " << recvPacket.message << endl;

					break;
				case MESSAGE_FLAG:
					cout << clientNick[recvPacket.id] << " : "
						<< recvPacket.message << endl;
					break;
				}

				recvBuf.buf = (char*)&recvPacket;
				recvBuf.len = LEN_ID_FLAGS_SIZE;
				memset(&overlapped, 0, sizeof(WSAOVERLAPPED));
				overlapped.hEvent = wsaEvent[HEADER_RECV_EVENT];
				if (WSARecv(clntSock, &recvBuf, 1, (DWORD*)&recvBytes, (DWORD *)&flags,
					&overlapped, NULL) == SOCKET_ERROR) {
					if (WSAGetLastError() != WSA_IO_PENDING)
						ErrorHandling("RecvThread RecvHeader3 Error");
				}
			}
			//메시지 다 못받았을 경우 추가 Recv
			else if (messageRecvBytes > recvPacket.len) {
				recvBuf.buf = recvPacket.message + messageRecvBytes;
				recvBuf.len = recvPacket.len - messageRecvBytes;
				memset(&overlapped, 0, sizeof(WSAOVERLAPPED));
				overlapped.hEvent = wsaEvent[MESSAGE_RECV_EVENT];

				if (WSARecv(clntSock, &recvBuf, 1, (DWORD*)&recvBytes, (DWORD *)&flags,
					&overlapped, NULL) == SOCKET_ERROR) {
					if (WSAGetLastError() != WSA_IO_PENDING)
						ErrorHandling("RecvThread RecvMessage2 Error");
				}
			}
			break;
		default:

			break;
		}


	}

}

void SendThread(SOCKET clntSock) {
	WSAEVENT wsaEvent;
	wsaEvent = WSACreateEvent();
	WSAOVERLAPPED overlapped;
	int sendBytes = 0;
	int currentSendBytes = 0;
	int flags = 0;
	int index = 0;
	int messageLen = 0;

	cin >> sendPacket.message;
	messageLen = strlen(sendPacket.message);

	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = wsaEvent;
	sendBuf.len = messageLen + LEN_ID_FLAGS_SIZE;
	sendBuf.buf = (char*)&sendPacket;
	sendPacket.len = messageLen;
	sendPacket.id = myId;
	sendPacket.flags = MESSAGE_FLAG;

	if (WSASend(clntSock, &sendBuf, 1, (DWORD*)&sendBytes, (DWORD)flags,
		&overlapped, NULL) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING)
			ErrorHandling("SendThread send1 Error");
	}

	while (true) {
		WSAWaitForMultipleEvents(1, &wsaEvent, false, WSA_INFINITE, false);
		WSAResetEvent(wsaEvent);
		WSAGetOverlappedResult(clntSock, &overlapped,
			(DWORD*)&sendBytes, false, (DWORD*)&flags);

		if (sendBytes == 0)
			ErrorHandling("SendThread send2 Error");

		currentSendBytes += sendBytes;
		if (sendBuf.len == currentSendBytes) {
			currentSendBytes = 0;

			// 본인이 입력한 채팅 메시지 출력
			cout << myNick << " : " << sendPacket.message << endl;
			cin >> sendPacket.message;
			messageLen = strlen(sendPacket.message);

			memset(&overlapped, 0, sizeof(overlapped));
			overlapped.hEvent = wsaEvent;
			sendBuf.len = messageLen + LEN_ID_FLAGS_SIZE;
			sendBuf.buf = (char*)&sendPacket;
			sendPacket.len = messageLen;
			//sendPacket.id = myId;
			sendPacket.flags = MESSAGE_FLAG;

			if (WSASend(clntSock, &sendBuf, 1, (DWORD*)&sendBytes, (DWORD)flags,
				&overlapped, NULL) == SOCKET_ERROR) {
				if (WSAGetLastError() != WSA_IO_PENDING)
					ErrorHandling("SendThread send1 Error");
			}
		}
		else if (sendBuf.len > currentSendBytes) {
			memset(&overlapped, 0, sizeof(overlapped));
			overlapped.hEvent = wsaEvent;
			sendBuf.len = messageLen + LEN_ID_FLAGS_SIZE - currentSendBytes;
			sendBuf.buf = (char*)&sendPacket + currentSendBytes;
			//sendPacket.len = messageLen;
			//sendPacket.id = myId;
			sendPacket.flags = MESSAGE_FLAG;

			if (WSASend(clntSock, &sendBuf, 1, (DWORD*)&sendBytes, (DWORD)flags,
				&overlapped, NULL) == SOCKET_ERROR) {
				if (WSAGetLastError() != WSA_IO_PENDING)
					ErrorHandling("SendThread send1 Error");
			}
		}
	}
}
