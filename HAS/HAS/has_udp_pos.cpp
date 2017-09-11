#include "has_udp_pos.h"



UDPServer::UDPServer() {

}

UDPServer::UDPServer(ClientState clients[]) {
	clientState = clients;
}

void UDPServer::serverStart() {
	serverUDPSock = socket(PF_INET, SOCK_DGRAM, 0);
	if (serverUDPSock == INVALID_SOCKET)
		ErrorHandling("socket()");

	memset(&serverUDPAddr, 0, sizeof(serverUDPAddr));
	serverUDPAddr.sin_family = AF_INET;
	serverUDPAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverUDPAddr.sin_port = htons(DEFAULT_PORT);

	if (::bind(serverUDPSock, (sockaddr*)&serverUDPAddr, sizeof(serverUDPAddr)) == SOCKET_ERROR)
		ErrorHandling("bind");
}

void UDPServer::receiveClientAddr() {
	int iResult = 0;
	char* tempFlag = new char[4];
	ZeroMemory(tempFlag, sizeof(int));
	for (int i = 0; i < CLIENT_MAX; i++) {
		iResult = recvfrom(serverUDPSock, tempFlag, sizeof(int), 0, (SOCKADDR*)&clientState[i].clientUDPAddr, &clientState[i].clientUDPAddrSize);
		if (iResult == SOCKET_ERROR) {
			int ErrCode = WSAGetLastError();
			if (ErrCode != WSA_IO_PENDING)
			{
				//cout << error << endl;
				cout << "error code : " << ErrCode << endl;
				//exit(1);
			}
			ErrorHandling("UDP recvfrom12");
		}
	}
	delete[]tempFlag;
}

void UDPServer::recvThreadStart() {
	//recvUDPPosThread = thread(recvPosThread, clientState, serverUDPSock);
	recvUDPPosThread = std::thread([&] {recvPosThreadMain(clientState, serverUDPSock); });
}

void UDPServer::sendThreadStart() {
	//sendUDPPosThread = thread(sendPosThread, clientState, serverUDPSock, connectNum);			//�� ��쿡�� Ŭ������ ����Լ��� static���� �����ؾ� �Ѵ�.
	sendUDPPosThread = std::thread([&] {sendPosThreadMain(clientState, serverUDPSock); });	//���� ���� �̿��Ͽ� Ŭ������ ��� �Լ��� thread�� �����Ų��.
}

void UDPServer::threadJoin() {
	if (recvUDPPosThread.joinable() == true) {
		recvUDPPosThread.join();
	}
	if (sendUDPPosThread.joinable() == true) {
		sendUDPPosThread.join();
	}
}

void UDPServer::serverClosed() {
	closesocket(serverUDPSock);
}

void UDPServer::clientsAddrInit()
{
	for (int i = 0; i < CLIENT_MAX; i++) {
		clientState[i].pos = { -1, -1, -1, -1, -1, -1 };
		ZeroMemory((char*)&clientState[i].clientUDPAddr, sizeof(clientState[i].clientUDPAddr));
		clientState[i].clientUDPAddrSize = sizeof(clientState[i].clientUDPAddr);
	}
}

int UDPServer::recvPosThreadMain(ClientState clientState[], SOCKET serverUDPSock) {
	//Ŭ���̾�Ʈ�� ��ǥ ������ �޾ƿ��� thread
	std::mutex mutex;

	int bufLen = sizeof(int) + sizeof(Pos);
	char* recvBuf = new char[bufLen];

	while (1) {
		SOCKADDR_IN temp;
		int tempAddrSize = sizeof(temp);
		int tempId = -1;
		int iResult = 0;
		ZeroMemory(recvBuf, bufLen);

		iResult = recvfrom(serverUDPSock, recvBuf, bufLen, 0, (SOCKADDR *)&temp, &tempAddrSize);
		if (iResult == -1) {
			//ErrorHandling("UDP recvfrom");
			cout << "EOF" << endl;
			break;
		}

		memcpy(&tempId, recvBuf, sizeof(int));

		mutex.lock();
		memcpy(&clientState[tempId].pos, recvBuf + sizeof(int), sizeof(Pos));
		mutex.unlock();
	}
	delete[]recvBuf;

	cout << "UDP recv thread end" << endl;

	return 0;
}

int UDPServer::sendPosThreadMain(ClientState clientState[], SOCKET serverUDPSock) {
	//Ŭ���̾�Ʈ�鿡�� ��ǥ ������ �����ϴ� thread
	std::mutex mutex;

	int bufLen = sizeof(int) + sizeof(Pos);
	char* sendBuf = new char[bufLen];
	int disconnectedNum = 0;

	while (true) {
		for (int i = 0; i < CLIENT_MAX; i++) {
			int iResult = 0;
			if (clientState[i].id != -1) {
				ZeroMemory(sendBuf, bufLen);
				memcpy(sendBuf, &clientState[i], sizeof(int) + sizeof(Pos));

				for (int j = 0; j < CLIENT_MAX; j++) {

					if (i != j) {
						iResult = sendto(serverUDPSock, sendBuf, bufLen, 0, (SOCKADDR*)&clientState[j].clientUDPAddr, clientState[j].clientUDPAddrSize);
						if (iResult == SOCKET_ERROR) {
							ErrorHandling("UDP sendto");
						}
					}
				}
			}
		}//for end

		//cout << "user count : " << *connectNum << endl;

		if (clientCount == 0) {
			cout << "[UDP]user count : " << clientCount << endl;
			//recv thread�� ���� ��Ű�� �ڵ� 
			SuspendThread(recvUDPPosThread.native_handle());		//�Ͻ� ����
			TerminateThread(recvUDPPosThread.native_handle(), 0);	//����
			break;
		}
		Sleep(10);
	}//while end

	delete[]sendBuf;
	cout << "UDP send thread end" << endl;
	return 0;
}