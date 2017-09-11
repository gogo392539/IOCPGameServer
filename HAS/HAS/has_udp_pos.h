#pragma once

#include "has_info.h"

class UDPServer {
private:
	SOCKET serverUDPSock;
	SOCKADDR_IN serverUDPAddr;
	ClientState* clientState;
	std::thread recvUDPPosThread;
	std::thread sendUDPPosThread;

public:
	UDPServer();
	UDPServer(ClientState clients[]);
	void serverStart();
	void receiveClientAddr();
	void recvThreadStart();
	void sendThreadStart();
	void threadJoin();
	void serverClosed();
	void clientsAddrInit();

	int recvPosThreadMain(ClientState clientState[], SOCKET serverUDPSock);
	int sendPosThreadMain(ClientState clientState[], SOCKET serverUDPSock);
};