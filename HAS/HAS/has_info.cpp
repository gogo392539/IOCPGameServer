#include "has_info.h"

int clientCount = 0;

void ErrorHandling(string error) {
	cout << error << " error" << endl;
	exit(1);
}

int sendn(SOCKET socket, const char *buf, int len, int flag) {
	int nleft;
	int nsendn;

	nleft = len;

	while (nleft > 0) {
		nsendn = send(socket, buf, nleft, flag);

		if (nsendn <= 0) {
			return (SOCKET_ERROR);        /* error */
		}

		nleft -= nsendn;
		buf += nsendn;
	}
	return (len - nleft);
}

int recvn(SOCKET socket, char* buf, int len, int flag) {
	int nleft;
	int nrecv;

	nleft = len;
	while (nleft > 0) {
		nrecv = recv(socket, buf, nleft, flag);

		if (nrecv < 0) {
			return (SOCKET_ERROR);        /* error */
		}
		else if (nrecv == 0) {
			break;						  /*EOF*/
		}

		nleft -= nrecv;
		buf += nrecv;
	}
	return (len - nleft);
}

void errorCodeCheck(int result, string error)
{
	if (SOCKET_ERROR == result)
	{
		int ErrCode = WSAGetLastError();
		if (ErrCode != WSA_IO_PENDING)
		{
			cout << error << endl;
			cout << "error code : " << ErrCode << endl;
			//exit(1);
		}
	}
}

