#pragma once
#include "interface/ezNetwork.h"
#include "msgInterface.h"
#include "sockets.h"
#include <stack>
#include <cassert>
#include <thread>
using namespace ezm;
using namespace internal;


struct Listener
{
	SOCKET socket;
	SOCKADDR_IN addr;
	std::atomic<bool> listening = true;

	Listener(unsigned port)
	{
		socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = INADDR_ANY;
		addr.sin_port = htons(port);

		setsockopt(socket, SOL_SOCKET, SO_BROADCAST, (char*)&addr, sizeof(addr));
		bind(socket, (PSOCKADDR)&addr, sizeof(addr));
		listen(socket, SOMAXCONN);
	}

	~Listener()
	{
		listening = false;
		shutdown(socket, SD_BOTH);
		closesocket(socket);
	}

	SOCKET waitForConnection()
	{
		int size = sizeof(addr);
		const auto clientSocket = accept(socket, (sockaddr*)&addr, &size);
		return clientSocket;
	}
	
};

struct ezServer : virtual internal::Server {
	std::list<Connection> connections;
	std::thread connection_thread;
	Listener listener;

	ezServer(unsigned port) : listener(port) {
		connection_thread = std::thread(&ezServer::acceptConnections, this);
		connection_thread.detach();
	}

	void acceptConnections()
	{
		while(listener.listening)
		{
			auto client = listener.waitForConnection();
			//connections.emplace_back(client, recieved, toSend);
		}
	}

};