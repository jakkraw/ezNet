#pragma once
#include <thread>
#include "queue.h"
#include "socket.h"
#include "serverFinder.h"
#include <functional>
#include "connection.h"

struct Connections {
	std::list<Connection> connetions;
	std::mutex mutex;
	std::atomic<size_t> counter;

	void add(TcpSocket&& s) {
		std::lock_guard<std::mutex> lock(mutex);
		connetions.emplace_back(std::move(s));
		counter = connetions.size();
	}

	void forEach(const std::function<void(Connection&)>& func) {
		std::lock_guard<std::mutex> lock(mutex);
		for (auto& connection : connetions)
			func(connection);
	}

	void deleteInvalid() {
		std::lock_guard<std::mutex> lock(mutex);
		connetions.remove_if([](const Connection& c) { return !c.isValid(); });
		counter = connetions.size();
	}

	size_t size() const {
		return counter;
	}

};

struct Listener
{
	TcpSocket socket;
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
					printf("connected port:%d ip:%s\n", client.second.port, client.second.ip.c_str()), connections.add(std::move((TcpSocket&)client.first));
			}
		} };

		deleter = std::thread{ [this]()
		{
			while (listening) {
				connections.deleteInvalid();
				std::this_thread::sleep_for(1000ms);
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
		connections.forEach([=](Connection& connection)
			{
				connection.send(msg);
			});
	}

	size_t nrOfConnections() const {
		return connections.size();
	}

	Address local() {
		return { "127.0.0.1", listener.port() };
	}

	template<typename Data>
	std::list<Data> recieve() {
		std::list<Data> list;
		connections.forEach([&list](Connection&connection)
			{
				list.splice(list.end(), connection.recieve<Data>());
			});
		return list;
	}

	template<typename Data>
	void broadcastAny() {
		std::list<Msg> list;
		connections.forEach([&list](Connection&connection)
		{
			list.splice(list.end(), connection.recieved.get<Data>());
		});

		for (auto& msg : list)
			connections.forEach([&msg](Connection& connection)
		{
			if (connection.id() != msg.sender())
				connection.send(msg);
		});
	}

};