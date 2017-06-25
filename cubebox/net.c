#include "net.h"

#include <stdio.h>
#include <string.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

void cbInitNet() {
    
}

void cbDestroyNet() {

}

cbSocket cbOpenSocket() {
    return socket(AF_INET, SOCK_STREAM, 0);
}

void cbSocketSetBlock(cbSocket s, bool block) {
    int opts = fcntl(s, F_GETFL);
	if (block) {
		opts = opts & (~O_NONBLOCK);
		fcntl(s, F_SETFL, opts);
	} else {
		fcntl(s, F_SETFL, opts | O_NONBLOCK);
	}
}

int cbSocketSelect(cbSocket s) {
    fd_set set;
    FD_ZERO(&set);
    FD_SET(s, &set);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    return select(s + 1, &set, NULL, NULL, &timeout);
}

int cbSocketConnect(cbSocket s, const char* addr, int port) {
    struct addrinfo hints;
    struct addrinfo *result, *iter;

    // if ip
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    if (inet_aton(addr, &address.sin_addr) == 1) {
        
        address.sin_port = htons(port);
        if (connect(s, (struct sockaddr*) &address, sizeof(address)) != 0) {
            return false;
        }
        return true;
    }

    // if host
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(addr, NULL, &hints, &result) != 0) {
        return false;
    }

    for (iter = result; iter; iter = iter->ai_next) {
        if (connect(s, iter->ai_addr, iter->ai_addrlen) != 0) {
            return false;
        }
    }

    freeaddrinfo(result);
    return true;
}

int cbSocketListen(cbSocket s, int port) {
	int yes = 1;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) != 0) {
		return false;
	}

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

	if (bind(s, (struct sockaddr*) &address, sizeof(address)) != 0) {
		return false;
	}
	if (listen(s, 0) != 0) {
		return false;
	}
}

int cbSocketRead(cbSocket s, char* buf, int len) {
    return recv(s, buf, len, 0); // no flags
}

int cbSocketWrite(cbSocket s, char* buf, int len) {
    return send(s, buf, len, MSG_NOSIGNAL); // prevent SIGPIPE
}

cbSocket cbSocketAccept(cbSocket s) {
    struct sockaddr_in address;
    int len = sizeof(address);

    return accept(s, (struct sockaddr*) &address, &len);
}

void cbCloseSocket(cbSocket s) {
    close(s);
}
