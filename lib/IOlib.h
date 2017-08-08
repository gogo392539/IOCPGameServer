#ifndef __IOlib_
#define __IOlib_

#include <Winsock2.h>

extern int sendn(SOCKET socket, const char *buf, int len, int flag);
extern int recvn(SOCKET socket, char* buf, int len, int flag);

#endif