#pragma once

#include "has_info.h"

class has_iocp_tcp
{
private:
	WSADATA wsaData;
	SOCKET hasListenSock;
	ClientState* clientState;
	SOCKADDR_IN hasListenAddr;
	SOCKADDR_IN hasClientAddr;
	int hasClientAddrsz;
	int taggerUserID;
	int nZero;
	eventPacket eventpacket;

	HANDLE hCompletionPort;
	PER_HANDLE_DATA* PerHandleData;
	PER_IO_DATA* PerIoData;
	/*DWORD RecvBytes;
	DWORD Flags;*/

public:
	has_iocp_tcp();
	~has_iocp_tcp();

	has_iocp_tcp(ClientState clients[]);
	void hasInit();
	void hasClosed();
	void hasUserConnection();

	void sendRandomIdx();
	void randomPos(int arrayPos[]);
	void sendTaggerUserID();
	int selectTaggerUser();

	//unsigned int __stdcall CompletionThread(HANDLE CompPort);
};

unsigned int __stdcall CompletionThread(HANDLE CompPort);