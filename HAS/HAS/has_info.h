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

//client�� ��ǥ ������ ���� ����
struct Pos {
	float x;
	float y;
	float z;
	float rotX;
	float rotY;
	float rotZ;
};

//Ŭ���̾�Ʈ�� ������ �����ϴ� ����ü
struct ClientState {
	int id;
	Pos pos;
	SOCKET clientTCPSock;
	SOCKADDR_IN clientUDPAddr;
	int clientUDPAddrSize;
};

//Ŭ���̾�Ʈ�� ����� ���� ��Ŷ ����
struct eventPacket {
	int flag;
	int id;
	int set;
};

//IOCP�� ���� �ڵ麰 ������
struct PER_HANDLE_DATA {
	SOCKET hasClientSock;
	SOCKADDR_IN hasClientAddr;
	ClientState* clients;
	int id;
	int *clientCount;
};

//IOCP�� ���� IO �۾��� ������
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
