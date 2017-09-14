#include "has_info.h"
#include "has_iocp_tcp.h"
#include "has_udp_pos.h"

int main(void) {

	//ClientState clientstate[CLIENT_MAX];
	/*for (int i = 0; i < CLIENT_MAX; i++) {
		clientstate[i].pos = { -1, -1, -1, -1, -1, -1 };
		ZeroMemory((char*)&clientstate[i].clientUDPAddr, sizeof(clientstate[i].clientUDPAddr));
		clientstate[i].clientUDPAddrSize = sizeof(clientstate[i].clientUDPAddr);
	}*/

	for (int i = 0; i < CLIENT_MAX; i++) {
		clients[i].pos = { -1, -1, -1, -1, -1, -1 };
		clients[i].clientUDPAddrSize = sizeof(clients[i].clientUDPAddr);
	}

	has_iocp_tcp hasIocpServer = has_iocp_tcp();
	hasIocpServer.hasInit();


	while (1) {
		cout << "--------------------------------------------------------------" << endl;
		cout << "Server Start" << endl;
		cout << endl;

		UDPServer hasUDPServer = UDPServer();
		hasUDPServer.serverStart();

		hasIocpServer.hasUserConnection();

		hasUDPServer.receiveClientAddr();
		hasUDPServer.recvThreadStart();
		hasIocpServer.sendRandomIdx();
		hasIocpServer.sendTaggerUserID();
		hasUDPServer.sendThreadStart();

		hasUDPServer.threadJoin();
	//	hasUDPServer.clientsAddrInit();

		hasUDPServer.serverClosed();

		cout << endl;
		cout << "server closed" << endl;
		cout << "--------------------------------------------------------------" << endl;

	}

	hasIocpServer.hasClosed();
	

	//IOCP thread 종료 방법 및 TRACE 함수 사용법 추가하기

	return 0;
}