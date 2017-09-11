#include "has_iocp_tcp.h"



has_iocp_tcp::has_iocp_tcp()
{
}


has_iocp_tcp::has_iocp_tcp(ClientState clients[])
{
	hasClientAddrsz = sizeof(hasListenAddr);
	clientState = clients;
	nZero = 0;
}

has_iocp_tcp::~has_iocp_tcp()
{

}

void has_iocp_tcp::hasInit()
{
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		ErrorHandling("WSAStartup error");
	}

	hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);

	for (int i = 0; i < systemInfo.dwNumberOfProcessors; i++)
		_beginthreadex(NULL, 0, CompletionThread, (LPVOID)hCompletionPort, 0, NULL);

	hasListenSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	memset(&hasListenAddr, 0, sizeof(hasListenAddr));
	hasListenAddr.sin_family = AF_INET;
	hasListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	hasListenAddr.sin_port = htons(DEFAULT_PORT);

	if (bind(hasListenSock, (SOCKADDR*)&hasListenAddr, sizeof(hasListenAddr))
		== SOCKET_ERROR) {
		ErrorHandling("bind error");
	}

	if (listen(hasListenSock, 5) == SOCKET_ERROR) {
		ErrorHandling("listen error");
	}
}

void has_iocp_tcp::hasUserConnection()
{
	DWORD Flags;
	DWORD RecvBytes;

	while (clientCount < CLIENT_MAX) {
		hasClientAddrsz = sizeof(hasClientAddr);
		clientState[clientCount].clientTCPSock = accept(hasListenSock,
			(SOCKADDR*)&hasClientAddr, &hasClientAddrsz);
		if (clientState[clientCount].clientTCPSock == INVALID_SOCKET) {
			ErrorHandling("accept");
		}

		clientState[clientCount].id = clientCount;
		cout << "connect" << clientCount << endl;

		int sendNum = sendn(clientState[clientCount].clientTCPSock,
			(char*)&clientState[clientCount].id, sizeof(clientState[clientCount].id), 0);
		errorCodeCheck(sendNum, "send id error");
		/*if (sendNum == SOCKET_ERROR) {
			ErrorHandling("ID send ");
		}*/

		/*
		setsockopt() 함수를 통해 send와 recv의 커널 단 버퍼의 크기를 0으로 만들고 있다.
		IOCP가 Overlapped IO에 대한 결과를 통보 받는 메커니즘이기 때문에 커널단 버퍼를 사용하지
		않고 직접 제공된 버퍼를 사용한다.
		*/
		nZero = 0;
		if (SOCKET_ERROR == setsockopt(clientState[clientCount].clientTCPSock, SOL_SOCKET, SO_RCVBUF,
			(const char*)&nZero, sizeof(int))) {
			ErrorHandling("setsockopt error");
		}

		nZero = 0;
		if (SOCKET_ERROR == setsockopt(clientState[clientCount].clientTCPSock, SOL_SOCKET, SO_SNDBUF,
			(const char*)&nZero, sizeof(int))) {
			ErrorHandling("setsockopt error");
		}

		u_long mode = 1;
		ioctlsocket(clientState[clientCount].clientTCPSock, FIONBIO, &mode);

		PerHandleData = new PER_HANDLE_DATA();
		PerHandleData->hasClientSock = clientState[clientCount].clientTCPSock;
		memcpy(&PerHandleData->hasClientAddr, &hasClientAddr, hasClientAddrsz);
		PerHandleData->id = clientCount;
		//PerHandleData->clientCount = userCount;
		//cout << "accept user count : " << *(PerHandleData->clientCount) << endl;
		PerHandleData->clients = clientState;

		CreateIoCompletionPort((HANDLE)clientState[clientCount].clientTCPSock, hCompletionPort,
			(DWORD)PerHandleData, 0);

		PerIoData = new PER_IO_DATA;
		memset(&PerIoData->overlapped, 0, sizeof(OVERLAPPED));
		PerIoData->wsaBuf.len = EVENTPACKET_SIZE;
		PerIoData->wsaBuf.buf = (char*)&PerIoData->eventpacket;

		Flags = 0;

		int result = WSARecv(clientState[clientCount].clientTCPSock, &PerIoData->wsaBuf, 1,
			&RecvBytes, &Flags, &PerIoData->overlapped, NULL);
		if (SOCKET_ERROR == result)
		{
			int ErrCode = WSAGetLastError();
			if (ErrCode != WSA_IO_PENDING)
			{
				cout << "line 120" << endl;
				cout << "error code : " << ErrCode << endl;
				//exit(1);
			}
		}

		clientCount++;

	}
}

void has_iocp_tcp::sendRandomIdx()
{
	int arrayPos[CLIENT_MAX];
	//randomPos(arrayPos);
	arrayPos[0] = 4;
	arrayPos[1] = 10;
	cout << "Raomdom Index : ";
	for (int i = 0; i < CLIENT_MAX; i++) {
		cout << arrayPos[i];
		if (i != CLIENT_MAX - 1) {
			cout << ", ";
		}
	}
	cout << endl;

	int dataLen = sizeof(arrayPos);
	char* posData = new char[dataLen];

	ZeroMemory(posData, dataLen);
	memcpy(posData, arrayPos, dataLen);

	for (int i = 0; i < CLIENT_MAX; i++) {
		//client의 시작위치를 보낸다.		
		int iResult = sendn(clientState[i].clientTCPSock, posData, dataLen, 0);
		if (SOCKET_ERROR == iResult)
		{
			int ErrCode = WSAGetLastError();
			if (ErrCode != WSA_IO_PENDING)
			{
				cout << "pos send error" << endl;
				cout << "error code : " << ErrCode << endl;
				//exit(1);
			}
		}
		Sleep(10);
	}
	delete[]posData;
}

void has_iocp_tcp::randomPos(int arrayPos[])
{
	srand((unsigned int)time(NULL));
	for (int i = 0; i < CLIENT_MAX; i++) {
		int tempPos = rand() % 15;
		arrayPos[i] = tempPos;

		for (int j = 0; j < i; j++) {
			if (arrayPos[i] == arrayPos[j]) {
				i--;
				break;
			}
		}
	}
}

void has_iocp_tcp::sendTaggerUserID()
{
	//모든 client에게 술래의 ID 값을 전달
	/*taggerUserID = SelectTaggerUser();

	for (int i = 0; i < CLIENT_MAX; i++) {
	int sendNum = sendn(clientState[i].clientTCPSock, (char*)&taggerUserID, sizeof(int), 0);
	}

	cout << "tagger user ID : " << taggerUserID << endl;*/

	int tempTagger = 0;
	for (int i = 0; i < CLIENT_MAX; i++) {
		int sendNum = sendn(clientState[i].clientTCPSock, (char*)&tempTagger, sizeof(int), 0);
	}
	cout << "tagger user ID : " << tempTagger << endl;
}

int has_iocp_tcp::selectTaggerUser()
{
	//tagger user ID 를 난수로 생성
	srand((unsigned int)time(NULL));
	int taggerID = rand() % CLIENT_MAX;

	return taggerID;
}

void has_iocp_tcp::hasClosed()
{
	/*for (int i = 0; i < CLIENT_MAX; i++) {
		closesocket(clientState[i].clientTCPSock);
	}*/
	closesocket(hasListenSock);
	WSACleanup();
}

//unsigned int __stdcall has_iocp_tcp::CompletionThread(HANDLE CompPortMem)
//{
//
//
//
//
//
//	return 0;
//}

unsigned int __stdcall CompletionThread(HANDLE CompPort)
{
	std::mutex mutex;
	HANDLE hCompletionPort = (HANDLE)CompPort;
	PER_HANDLE_DATA* PerHandleData;
	PER_IO_DATA* PerIoData;
	DWORD BytesTransferred;
	DWORD flags;

	eventPacket packet;

	while (1) {
		GetQueuedCompletionStatus(hCompletionPort, &BytesTransferred, (LPDWORD)&PerHandleData,
			(LPOVERLAPPED*)&PerIoData, INFINITE);
		if (BytesTransferred == 0) {
			//client 종료
			cout << "user#" << PerHandleData->id << " closed!" << endl;
			mutex.lock();
			clientCount--;
			PerHandleData->clients[PerHandleData->id].id = -1;
			mutex.unlock();
			cout << "user count : " << clientCount << endl;
			closesocket(PerHandleData->hasClientSock);
			closesocket(PerHandleData->clients[PerHandleData->id].clientTCPSock);
			PerHandleData->clients[PerHandleData->id].clientTCPSock = INVALID_SOCKET;
			delete PerHandleData;
			delete PerIoData;
			continue;
		//	break;
		}

		if (PerIoData->eventpacket.flag == 2) {
			cout << "killed ID : " << PerIoData->eventpacket.id << endl;
		}

		memcpy(&packet, &PerIoData->eventpacket, EVENTPACKET_SIZE);
		for (int i = 0; i < CLIENT_MAX; i++) {
			if (i != PerHandleData->id && 
				PerHandleData->clients[PerHandleData->id].clientTCPSock != INVALID_SOCKET) {
				sendn(PerHandleData->clients[i].clientTCPSock, (char*)&packet, 
					EVENTPACKET_SIZE, 0);
			}
		}

		memset(&PerIoData->overlapped, 0, sizeof(OVERLAPPED));
		memset(&PerIoData->eventpacket, 0, sizeof(PerIoData->eventpacket));
		PerIoData->wsaBuf.len = EVENTPACKET_SIZE;
		PerIoData->wsaBuf.buf = (char*)&(PerIoData->eventpacket);

		flags = 0;

		int result = WSARecv(PerHandleData->hasClientSock, &PerIoData->wsaBuf, 1, NULL, &flags,
			&PerIoData->overlapped, NULL);
		if (SOCKET_ERROR == result)
		{
			int ErrCode = WSAGetLastError();
			if (ErrCode == WSA_IO_PENDING)
			{
				
				//exit(1);
			}
			else if (ErrCode == WSAECONNRESET) {
				cout << "user#" << PerHandleData->id << " closed!" << endl;
				mutex.lock();
				clientCount--;
				PerHandleData->clients[PerHandleData->id].id = -1;
				mutex.unlock();
				cout << "user count : " << clientCount << endl;
				closesocket(PerHandleData->hasClientSock);
				closesocket(PerHandleData->clients[PerHandleData->id].clientTCPSock);
				PerHandleData->clients[PerHandleData->id].clientTCPSock = INVALID_SOCKET;
				delete PerHandleData;
				delete PerIoData;
			}
			else {
				cout << "line 277" << endl;
				cout << "error code : " << ErrCode << endl;
			}
		}

	} 

	return 0;
}
