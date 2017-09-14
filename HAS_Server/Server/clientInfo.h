#pragma once

#include <iostream>
#include <WinSock2.h>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <ctime>
#include <string>

#pragma comment (lib, "Ws2_32.lib")

using namespace std;

#define CLIENT_MAX 5
#define DEFAULT_PORT 36872
#define PUZZLE_MAX 8
#define TRAP_MAX 5

#define EVENTPACKET_SIZE 12

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

struct eventPacket {
	int flag;
	int id;	//퍼즐/함정 이벤트의 경우 해당 퍼즐/함정의 ID값
	int Set;
};

void ErrorHandling(string);
int sendn(SOCKET socket, const char *buf, int len, int flag);
int recvn(SOCKET socket, char* buf, int len, int flaf);