#pragma once


#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include "interface/types.h"
#pragma comment(lib, "Ws2_32.lib")


struct Socket {
	enum class Type {
		TCP , UDP
	};

	SOCKET socket;

	Socket(Type type) {

		if (type == Type::TCP)
			socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		else
			socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	}

	void setBroadcast(bool enable) {
		auto err = setsockopt(socket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char *>(&enable), sizeof(enable));
	}

	void setReusable(bool enable) {
		auto err = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char *>(&enable), sizeof(enable));
	}


	bool bind(const lan::Port& port = 0, const lan::IP& ip = "0.0.0.0") {

		sockaddr_in addr = sockaddr_in();
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = inet_addr(ip.c_str());

		auto err = ::bind(socket, (PSOCKADDR)&addr, sizeof(addr));
		if (err) {}
	}





	~Socket() {
		closesocket(socket);
	}


};