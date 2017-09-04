#pragma once

#include "clientInfo.h"

class TCPServer {
private:
	WSADATA wsaData;
	int *connectNum;							//접속한 client의 수를 나타내기 위한 변수
	SOCKET serverListenSock;
	ClientState* clientState;
	SOCKADDR_IN serverTCPAddr;
	SOCKADDR_IN clientTCPAddr;
	int clientTCPAddrsz;

	int taggerUserID;
	
	thread TCPThread;
	thread Client_Thread[CLIENT_MAX];

	eventPacket eventpacket;

public:
	TCPServer();
	TCPServer(ClientState clients[], int *clientNum);
	void serverStart();
	void clientAccept();
	
	void ServerClosed();
	void sendRandomIdx();
	void randomPos(int arrayPos[]);
	void sendTaggerUserID();
	int SelectTaggerUser();

	void ThreadJoin();
	void ThreadStart();
	int ThreadFunc();
	int ClientMainThread(int myID);

	void puzzleEventFunc(eventPacket packet, int ID);
	void playerkillEventFunc(eventPacket packet, int ID);
	void animationEventFunc(eventPacket packet);
	void trapEventFunc(eventPacket packet, int ID);
};