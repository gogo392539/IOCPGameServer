#include "totalserver.h"

ControlServer::ControlServer(){
	for (int i = 0; i < CLIENT_MAX; i++) {
		clientState[i].pos = { -1, -1, -1, -1, -1, -1 };
		clientState[i].clientUDPAddrSize = sizeof(clientState[i].clientUDPAddr);
	}
	connectNum = 0;

	TcpServer = TCPServer(clientState, &connectNum);
	UdpServer = UDPServer(clientState, &connectNum);
}

void ControlServer::createServer(){
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		ErrorHandling("WSAStartup() error");
	}

	//TcpServer = TCPServer(clientState, &connectNum);
	TcpServer.serverStart();							//TCP server 설정
	cout << "TCP start" << endl;	//

	//UdpServer = UDPServer(clientState, &connectNum);
	UdpServer.serverStart();							//UDP server 설정

	cout << "TCP accept" << endl;	//
	TcpServer.clientAccept();							//TCP server가 client의 접속 요청을 받아들이고 client들에게 ID값을 전달
	//connectNum = TCPserver.getConnectNum();			//접속한 client들의 수를 반환	
	cout << "connectNum : " << connectNum << endl;

	UdpServer.receiveClientAddr();			//UDP server가 client들과 통신을 위해 client들의 주소값을 저장하고 sync를 맞추는 부분



	//UDP thread
	UdpServer.recvUDPThreadFunc();						//UDP server가 client들의 좌표정보를 받는 thread
	TcpServer.sendRandomIdx();							//client들에게 random index전달
	UdpServer.sendUDPThreadFunc();			//UDP server가 client들로부터 받은 좌표정보를 전달하는 thread

	TcpServer.clientStateTCPThreadFunc();

	TcpServer.tempThreadStart();
	
}


void ControlServer::closedServer() {
	TcpServer.TCPThreadJoin();
	UdpServer.UDPThreadJoin();
	cout << "close1" << endl;
	TcpServer.TCPServerClosed();
	UdpServer.UDPServerClosed();
	cout << "close2" << endl;

	WSACleanup();
}
