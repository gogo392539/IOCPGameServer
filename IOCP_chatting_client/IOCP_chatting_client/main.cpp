#include <iostream>
#include <algorithm>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "../../lib/IOlib.h"
#include "../../lib/IOlib.cpp"

#define PORT 20001
#define LEN_ID_SIZE 3
#define NICK_MAX_LEN 32
#define CLIENT_MAX 5

using namespace std;

void ErrorHandling(char* msg);


//2017.08.07 손기문 맨 처음 id 할당받을때 이 구조체의 id로 받아옴
#pragma pack(push, 1)   
struct chatPacket {
	char flag;
	char len;
	char id;
	char message[256];
};
#pragma pack(pop)

chatPacket recvPacket;
chatPacket sendPacket;
WSABUF recvBuf;
WSABUF sendBuf;
char myNick[32];
int myId = 0;
char clientNick[5][32];

void CALLBACK IdAllocRoutine(DWORD error, DWORD cbTramsferred, 
	LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);

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

	char myNick[32];
	cout << "닉네임을 입력하세요 : ";
	cin >> myNick;
	int nickLen = strlen(myNick);
	sendPacket.flag = 1;
	sendPacket.id = -1;
	sendPacket.len = nickLen;
	copy(myNick, myNick + nickLen, sendPacket.message);

	// 닉네임 보내기
	int flags = 0;
	
	if (sendn(clntSock, (char*)&sendPacket, sendPacket.len + LEN_ID_SIZE, flags)
		== SOCKET_ERROR)
		ErrorHandling("NICK send Error!");

	recvn(clntSock, (char*)&recvPacket, 3, 0);
	cout << (int)recvPacket.flag << ", " << (int)recvPacket.id << " ," << (int)recvPacket.len << endl;


	// 본인 아이디 + 다른 클라이언트 아이디, 닉네임 받기
	/*if (recvn(clntSock, (char*)&recvPacket, 2, flags) == SOCKET_ERROR)
		ErrorHandling("IDLEN recv Error!");
	myId = recvPacket.id;
	int recvLen = recvPacket.len;
	if (recvn(clntSock, (char*)&clientNick, recvLen, flags) == SOCKET_ERROR)
		ErrorHandling("client NINC recv Error!");*/

	//WSAEVENT wsaEvent = WSACreateEvent();
	//WSAOVERLAPPED overlapped;
	//memset(&overlapped, 0, sizeof(overlapped));
	//overlapped.hEvent = wsaEvent;
	//sendBuf.len = nickLen + LEN_ID_SIZE;
	//sendBuf.buf = (char* )&sendPacket;

	//int sendBytes = 0;
	//int flags = 0;
	//if (WSASend(clntSock, &sendBuf, 1, (DWORD *)&sendBytes,
	//	(DWORD)&flags, &overlapped, IdAllocRoutine) == SOCKET_ERROR) {
	//	if (WSAGetLastError() != WSA_IO_PENDING)
	//		ErrorHandling("nickSend Error");
	//}

	//int result = WSAWaitForMultipleEvents(1, &wsaEvent, true, WSA_INFINITE, true);
	////WSAGetOverlappedResult(clntSock, &overlapped, (DWORD* )&sendBytes, true, )
	//if (result != WAIT_IO_COMPLETION)
	//	ErrorHandling("nickSendWait Error!");






	return 0;
}	

void ErrorHandling(char* msg) {
	cout << msg << endl;
	exit(1);
}

void CALLBACK IdAllocRoutine(DWORD error, DWORD cbTramsferred,
	LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags) {


}