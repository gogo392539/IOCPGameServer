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
	TcpServer.serverStart();							//TCP server ����
	cout << "TCP start" << endl;	//

	//UdpServer = UDPServer(clientState, &connectNum);
	UdpServer.serverStart();							//UDP server ����

	cout << "TCP accept" << endl;	//
	TcpServer.clientAccept();							//TCP server�� client�� ���� ��û�� �޾Ƶ��̰� client�鿡�� ID���� ����
	//connectNum = TCPserver.getConnectNum();			//������ client���� ���� ��ȯ	
	cout << "connectNum : " << connectNum << endl;

	UdpServer.receiveClientAddr();			//UDP server�� client��� ����� ���� client���� �ּҰ��� �����ϰ� sync�� ���ߴ� �κ�



	//UDP thread
	UdpServer.recvUDPThreadFunc();						//UDP server�� client���� ��ǥ������ �޴� thread
	TcpServer.sendRandomIdx();							//client�鿡�� random index����
	UdpServer.sendUDPThreadFunc();			//UDP server�� client��κ��� ���� ��ǥ������ �����ϴ� thread

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
