#pragma once

#include "clientInfo.h"

class UDPServer {
private:
	SOCKET serverUDPSock;
	SOCKADDR_IN serverUDPAddr;
	ClientState* clientState;
	thread recvUDPPosThread;
	thread sendUDPPosThread;
	int *connectNum;

public:
	UDPServer();
	UDPServer(ClientState clients[], int *clientNum);
	void serverStart();
	void receiveClientAddr();
	void recvThreadStart();
	void sendThreadStart();
	void threadJoin();
	void serverClosed();

	int recvPosThreadMain(ClientState clientState[], SOCKET serverUDPSock);
	int sendPosThreadMain(ClientState clientState[], SOCKET serverUDPSock);
};