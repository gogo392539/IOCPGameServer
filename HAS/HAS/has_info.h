#pragma once

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <mutex>
#include <string>
#include <process.h>
#include <algorithm>
#include <thread>

#include <WinSock2.h>

#pragma comment (lib, "Ws2_32.lib")

#define CLIENT_MAX 2
#define PUZZLE_MAX 8
#define TRAP_MAX 5
#define DEFAULT_PORT 36872

#define EVENTPACKET_SIZE 12		

using std::string;
using std::cout;
using std::endl;

extern int clientCount;

//client의 좌표 정보와 각도 정보
struct Pos {
	float x;
	float y;
	float z;
	float rotX;
	float rotY;
	float rotZ;
};

//클라이언트의 정보를 저장하는 구조체
struct ClientState {
	int id;
	Pos pos;
	SOCKET clientTCPSock;
	SOCKADDR_IN clientUDPAddr;
	int clientUDPAddrSize;
};

//클라이언트와 통신을 위한 패킷 정의
struct eventPacket {
	int flag;
	int id;
	int set;
};

//IOCP를 위한 핸들별 데이터
struct PER_HANDLE_DATA {
	SOCKET hasClientSock;
	SOCKADDR_IN hasClientAddr;
	ClientState* clients;
	int id;
	int *clientCount;
};

//IOCP를 위한 IO 작업별 데이터
struct PER_IO_DATA {
	OVERLAPPED overlapped;
	WSABUF wsaBuf;
	//WSABUF wsaSendBuf;
	eventPacket eventpacket;
	//eventPacket sendPacket;
};

void ErrorHandling(string);
int sendn(SOCKET socket, const char *buf, int len, int flag);
int recvn(SOCKET socket, char* buf, int len, int flaf);
void errorCodeCheck(int result, string error);
