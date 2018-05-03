#pragma once
#include <thread>
#include "Queue.h"
#include "socket.h"
#include "serverFinder.h"
#include <functional>
#include "Connection.h"


struct Connections {
	std::list<Connection> connetions;
	std::mutex m;

	void add(Socket&& s) {
		std::lock_guard<std::mutex> lock(m);
		connetions.emplace_back(std::move(s));
	}

	void forEach(const std::function<void(Connection&)>& func) {
		std::lock_guard<std::mutex> lock(m);
		for (auto& connection : connetions)
			func(connection);
	}

	void deleteInvalid() {
		std::lock_guard<std::mutex> lock(m);
		connetions.remove_if([](const Connection& c) { return !c.isValid(); });
	}

};

struct Listener
{
	Socket socket{ Socket::Type::TCP };
	std::atomic<bool> listening = true;
	Connections& connections;
	std::thread inserter, deleter;

	Listener(Connections& c) : connections(c){
		socket.bind();
		socket.listen();
		inserter = std::thread{ [this]()
		{
			while (listening) {
				auto client = socket.accept();
				if (client.first.isValid())
					printf("connected port:%d ip:%s\n", client.second.port, client.second.ip.c_str()), connections.add(std::move(client.first));
			}
		} };

		deleter = std::thread{ [this]()
		{
			while (listening) {
				connections.deleteInvalid();
				std::this_thread::sleep_for(16ms);
			}
		} };
	}

	Port port() { return socket.address().port; }

	~Listener(){
		listening = false;
		socket.shutdown();
		socket.close();
		inserter.join();
		deleter.join();
	}
};

struct Server {
	Connections connections;
	Listener listener;
	Broadcaster broadcaster;

	Server() : listener(connections), broadcaster(listener.port()) {}

	template<typename Data>
	void send(const Data& msg) {
		connections.forEach([=](Connection&c){ c.send(msg); });
	}

	template<typename Data>
	std::list<Data> recieve() {
		std::list<Data> list;
		connections.forEach([&list](Connection&c) { list.splice(list.end(), c.recieve<Data>()); });
		return list;
	}
};