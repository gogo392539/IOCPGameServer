#include <iostream>
#include <algorithm>
#include <WinSock2.h>
#include <WS2tcpip.h>

#define PORT 20001
#define LEN_ID_SIZE 2

using namespace std;

void ErrorHandling(char* msg);


//2017.08.07 �ձ⹮ �� ó�� id �Ҵ������ �� ����ü�� id�� �޾ƿ�
#pragma pack(push, 1)   
struct chatPacket {
	char len;
	char id;
	char message[256];
};
#pragma pack(pop)

chatPacket recvPacket;
chatPacket sendPacket;
WSABUF recvBuf;
WSABUF sendBuf;

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
	cout << "�г����� �Է��ϼ��� : ";
	cin >> myNick;
	int nickLen = strlen(myNick);
	sendPacket.id = -1;
	sendPacket.len = nickLen;
	copy(myNick, myNick + nickLen, sendPacket.message);

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