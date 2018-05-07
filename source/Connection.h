#pragma once
#include "queue.h"
#include "socket.h"
#include "address.h"
#include <atomic>
#include <chrono>
using namespace std::chrono_literals;

struct Connection {
	Queue toSend, recieved;
	TcpSocket socket;
	std::atomic<bool> active{ true };
	std::thread sender{ [this]()
	{
		while (active)
		{
			for (auto&& msg : toSend.get())
				socket.send(msg);

			std::this_thread::sleep_for(0ms);
		}
	} };

	std::thread reciever{ [this]()
	{
		while (this->active) {
			auto package = this->socket.recieve();
			if (package.valid)
				this->recieved.add(std::move(package.msg));
		}

	} };

	static TcpSocket fromAddress(const Address& address) {
		TcpSocket socket;
		socket.connect(address);
		return socket;
	}

	Connection(const Address& address) : 
	socket(fromAddress(address)) {}

	Connection(TcpSocket&& socket) :
		socket(std::move(socket)) {}

	Connection(Connection&& connection) :
		socket(std::move(connection.socket)) {}


	~Connection() {
		active = false;
		socket.shutdown();
		socket.close();
		sender.join();
		reciever.join();
	}

	void send(Msg msg) {
		toSend.add(std::move(msg));
	}

	template<typename Data>
	void send(const Data& data) {
		toSend.add(Msg::toMsg(data));
	}

	bool isValid() const { return socket.isValid(); }
	size_t id() const  {
		return socket.socket;
	}

	template<typename Data>
	std::list<Data> recieve() {
		const auto msgs = recieved.get<Data>();
		std::list<Data> msgs1;
		for (const auto& msg : msgs)
			msgs1.emplace_back(msg.payloadAs<Data>());
		return msgs1;
	}
};
