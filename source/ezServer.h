#pragma once
#include "interface/ezNetwork.h"
#include "connection.h"
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
	MsgQueue toSend, recieved;
	std::list<Connection> connections;
	std::thread connection_thread;
	Listener listener;

	ezServer(Port port) : listener(port) {
		connection_thread = std::thread(&ezServer::acceptConnections, this);
		connection_thread.detach();
	}

	void acceptConnections()
	{
		while(listener.listening)
		{
			auto client = listener.waitForConnection();
			printf("connected port %d\n", client);
			connections.emplace_back(client, recieved, toSend);
			
		}
	}


	void _send(const Size& size, const ID& id, DataPtr data) override {
		toSend.add({size,id,data});
	}

	void _recieve(const ID& id, IVector& target) override {
		const auto msgs = recieved.get(id);
		target.reserve(msgs.size());
		for (const auto& msg : msgs)
			target.emplace_back(msg.payload());
	}

};