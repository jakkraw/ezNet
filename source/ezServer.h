#pragma once
#include "ezNetwork.h"
#include "msgInterface.h"
#include "sockets.h"
#include <memory>
#include <stack>
#include <cassert>
#include <thread>
using namespace ezNetwork;
using namespace internal;


struct Listener
{
	SOCKET socket;
	SOCKADDR_IN addr;

	Listener(Address::Port port)
	{
		socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = INADDR_ANY;
		addr.sin_port = htons(port);

		setsockopt(socket, SOL_SOCKET, SO_BROADCAST, (char*)&addr, sizeof(addr));
		bind(socket, (PSOCKADDR)&addr, sizeof(addr));
		listen(socket, SOMAXCONN);
	}

	Connection waitForConnection()
	{
		int size = sizeof(addr);
		auto clientSocket = accept(socket, (sockaddr*)&addr, &size);
		return { clientSocket };
	}
	
};

struct ezServer : private MsgInterface, Server {
	std::list<Connection> connections;
	std::thread connection_thread;

	Address address;

	ezServer(Address address) : address(address) {
		connection_thread = std::thread(&ezServer::acceptConnections, this);
	}

	void acceptConnections()
	{
		Listener listener(DefaultAddress.port);

		while(true)
		{
			auto client = listener.waitForConnection();
			connections.push_back(client);
		}

		/*
		tcpSocket s;
		s.bind();
		s.listen(10);

		while(acceptingConnections){
			auto c = s->accept();
			auto clientData = c->recvBlocking<ConnectionData>();
			
			ns(my addr ,port , other addr,port);
			


		}

		 
		 */
	}

};