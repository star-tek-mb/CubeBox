#ifndef CB_NET_H
#define CB_NET_H

#include <stdbool.h>

typedef int cbSocket;

void cbInitNet();
void cbDestroyNet();

cbSocket cbOpenSocket();
void cbSocketSetBlock(cbSocket s, bool block);
int cbSocketSelect(cbSocket s);
int cbSocketConnect(cbSocket s, const char* addr, int port);
int cbSocketListen(cbSocket s, int port);
cbSocket cbSocketAccept(cbSocket s);
int cbSocketRead(cbSocket s, char* buf, int len);
int cbSocketWrite(cbSocket s, char* buf, int len);
void cbCloseSocket(cbSocket s);

#endif
