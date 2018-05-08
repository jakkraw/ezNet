#pragma once
#include <functional>
#include <thread>
#include "ConcurrentMap.h"
#include "socket.h"
#include "serverFinder.h"
#include "connection.h"

struct Connections {
	
	void add(TcpSocket&& s) {
		std::lock_guard<std::mutex> lock(mutex);
		connetions.emplace_back(std::move(s));
		counter = connetions.size();
	}

	void forEach(const std::function<void(Connection&)>& func) {
		std::lock_guard<std::mutex> lock(mutex);
		for (auto& connection : connetions) func(connection);
	}

	void deleteInvalid() {
		std::lock_guard<std::mutex> lock(mutex);
		connetions.remove_if([](const Connection& c) { return !c.isValid(); });
		counter = connetions.size();
	}

	size_t size() const { return counter; }

	private:
		std::list<Connection> connetions;
		std::mutex mutex;
		std::atomic<size_t> counter;
};

struct Listener {
	
	Port port() { return socket.address().port; }

	Listener(Connections& c)
		: connections(c) {
		socket.bind();
		socket.listen();
		inserter = std::thread{
			[this]()
		{
			while (listening)
			{
				auto client = socket.accept();
				if (client.first.isValid()) printf("connected port:%d ip:%s\n", client.second.port, client.second.ip.c_str()),
					connections.add(std::move((TcpSocket&)client.first));
			}
		}
		};

		deleter = std::thread{
			[this]()
		{
			while (listening)
			{
				connections.deleteInvalid();
				std::this_thread::sleep_for(1000ms);
			}
		}
		};
	}

	~Listener() {
		listening = false;
		socket.shutdown();
		socket.close();
		inserter.join();
		deleter.join();
	}
	private:
		TcpSocket socket;
		std::atomic<bool> listening = true;
		Connections& connections;
		std::thread inserter, deleter;
};

struct Server {

	template <typename Data> void send(const Data& msg) {
		connections.forEach([=](Connection& connection) { connection.send(msg); });
	}

	template <typename Data> std::list<Data> recieve() {
		std::list<Data> list;
		connections.forEach([&list](Connection& connection) { list.splice(list.end(), connection.recieve<Data>()); });
		return list;
	}

	template <typename Data> void sendBroadcast() {
		std::list<Msg> msgs;
		connections.forEach([&msgs](Connection& connection) { msgs.splice(msgs.end(), connection.recieved.get<Data>()); });

		for (auto& msg : msgs)
			connections.forEach([&msg](Connection& connection) { if (connection.id() != msg.sender()) connection.send(msg); });
	}

	size_t connected() const { return connections.size(); }

	Address local() { return {"127.0.0.1", listener.port()}; }

	


	Server()
		: listener(connections),
		broadcaster(listener.port()) {}

	private:
		Connections connections;
		Listener listener;
		Broadcaster broadcaster;
};
