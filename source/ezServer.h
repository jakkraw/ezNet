#pragma once
#include "interface/ezNetwork.h"
#include "Connectione.h"
#include <stack>
#include <cassert>
#include <thread>
using namespace lan;
using namespace internal;


struct Informator {
	std::atomic<bool> running = true;
	std::thread searcher{ &Informator::searchThread, this };
	const Port port;
	Informator(const Port& port): port(port) {}

	~Informator() {
		running = false;
		searcher.join();
	}

	void searchThread() {
		
		auto socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		bool l = true;
		auto err = setsockopt(socket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char *>(&l), sizeof(l));
		err = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char *>(&l), sizeof(l));

		sockaddr_in addr = sockaddr_in();
		addr.sin_family = AF_INET;
		addr.sin_port = htons(19246);
		addr.sin_addr.s_addr = INADDR_ANY;

		err = ::bind(socket, (PSOCKADDR)&addr, sizeof(addr));
		if(err){}

		while (running)
		{
			sockaddr_in sender;
			int size = sizeof(sender);
			//char buffer[]{ 1 };
			auto res = recvfrom(socket, nullptr, 0, 0, (SOCKADDR *)& sender, &size);
			if (res > SOCKET_ERROR) {
				auto res = sendto(socket, (char*)&port, sizeof(port), 0, (SOCKADDR *)& sender, size);
			}
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(0s);
		}
	}

};

struct Listener
{
	SOCKET socket;
	SOCKADDR_IN addr;
	std::atomic<bool> listening = true;

	Listener()
	{
		socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		addr = sockaddr_in();
		addr.sin_family = AF_INET;
		addr.sin_port = 0;
		addr.sin_addr.s_addr = INADDR_ANY;

		bool enable{ true };
		setsockopt(socket, SOL_SOCKET, SO_BROADCAST, (char*)&enable, sizeof(enable));
		bind(socket, (PSOCKADDR)&addr, sizeof(addr));
		listen(socket, SOMAXCONN);
	}

	Port port() {
		sockaddr_in addr;
		int size = sizeof(addr);
		getsockname(socket, (sockaddr*)&addr, &size);

		printf("Port %d ip %s", ntohs(addr.sin_port), inet_ntoa(addr.sin_addr));

		return ntohs(addr.sin_port);
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

struct EzServer : virtual internal::Server {
	MsgQueue toSend, recieved;
	std::list<::Connectione> connections;
	std::thread connection_thread;
	Listener listener;
	Informator informator;

	EzServer() : listener(), informator(listener.port()) {
		connection_thread = std::thread(&EzServer::acceptConnections, this);
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