#pragma once

#include "clientInfo.h"
#include "classTcp.h"
#include "classUdp.h"

class ControlServer {
private:
	WSAData wsaData;
	bool serverSet;
	ClientState clientState[CLIENT_MAX];
	int connectNum;
	TCPServer TcpServer;
	UDPServer UdpServer;
	
public:
	ControlServer();
	void createServer();
	void setServerSet(bool set);
	void closedServer();
};
