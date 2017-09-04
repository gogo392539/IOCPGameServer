#include "classTcp.h"
#include "classUdp.h"
#include "totalserver.h"

int main()
{

	while (1) {
		ClientState clientState[CLIENT_MAX];
		for (int i = 0; i < CLIENT_MAX; i++) {
			clientState[i].pos = { -1, -1, -1, -1, -1, -1 };
			clientState[i].clientUDPAddrSize = sizeof(clientState[i].clientUDPAddr);
		}
		int connectNum = 0;

		TCPServer TCPserver = TCPServer(clientState, &connectNum);
		TCPserver.serverStart();				//TCP server ����
		cout << "--------------------------------------------------------------" << endl;
		cout << "Server Start" << endl;	
		cout << endl;

		UDPServer UDPserver = UDPServer(clientState, &connectNum);
		UDPserver.serverStart();				//UDP server ����

		TCPserver.clientAccept();				//TCP server�� client�� ���� ��û�� �޾Ƶ��̰� client�鿡�� ID���� ����

		UDPserver.receiveClientAddr();			//UDP server�� client��� ����� ���� client���� �ּҰ��� �����ϰ� sync�� ���ߴ� �κ�

		UDPserver.recvThreadStart();			//UDP server�� client���� ��ǥ������ �޴� thread
		TCPserver.sendRandomIdx();				//client�鿡�� random index����
		TCPserver.sendTaggerUserID();
		UDPserver.sendThreadStart();			//UDP server�� client��κ��� ���� ��ǥ������ �����ϴ� thread

		TCPserver.ThreadStart();

		UDPserver.threadJoin();
		cout << "UDP thread end" << endl;

		TCPserver.ThreadJoin();
		cout << "TCP thread end" << endl;

		TCPserver.ServerClosed();
		UDPserver.serverClosed();

		cout << endl;
		cout << "server closed" << endl;
		cout <<"--------------------------------------------------------------"<< endl;
	}
	
	return 0;
}

