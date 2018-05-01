#pragma once
#include <thread>
#include "msgQueue.h"
#include "socket.h"
#include "serverFinder.h"

struct Listener
{
	Socket socket{ Socket::Type::TCP };
	std::atomic<bool> listening = true;
	std::list<Socket> clients;
	std::condition_variable cv;
	std::mutex m;
	std::thread thread;

	Listener(){
		socket.bind();
		socket.listen();
		thread = std::thread{ [this]()
		{
			while (listening) {
				auto client = socket.accept();
				if (client.isValid()) {
					{
						std::lock_guard<std::mutex> lock(m);
						clients.emplace_back(std::move(client));
					}
					cv.notify_all();
				}
			}
		} };
	}

	Port port() { return socket.address().port; }

	~Listener(){
		listening = false;
		socket.shutdown();
		thread.join();
	}

	std::list<Socket> getAnyClients() {
		std::unique_lock<std::mutex> lock(m);
		cv.wait(lock, [this] {return !clients.empty(); });
		return std::move(clients);
	};

	std::list<Socket> getClients() {
		std::lock_guard<std::mutex> lock(m);
		return std::move(clients);
	};
	
};

struct EzServer {
	MsgQueue toSend, recieved;
	std::list<Socket> sockets;
	std::thread connection_thread, reciever;
	Listener listener;
	Broadcaster broadcaster;
	std::atomic<bool> active;

	EzServer() : broadcaster(listener.port()) {
		connection_thread = std::thread{ [this]()
			{
				while (active)
				{
					for (auto&& client : listener.getClients()) {
						printf("connected port %d\n", client.address().port);
						sockets.emplace_back(std::move(client));
					}

					for (auto&& entry : toSend.get())
						for (auto&& msg : entry.second)
							for (auto& socket : sockets)
								socket.send(msg);
				}
			} };

		reciever = std::thread{ [this]()
		{
			while (active)
			{
				for (auto& socket : sockets){
					auto package = socket.recieve();
					if(package.valid) recieved.add(std::move(package.msg));
				}
				
			}
		} };

	}

	~EzServer() {
		active = false;
		connection_thread.join();
		reciever.join();
	}


	template<typename Data>
	void send(const Data& msg) {
		toSend.add({ Msg::size<Data>(), Msg::type<Data>(), (Msg::Data)&msg });
	}

	template<typename Data>
	std::vector<Data> recieve() {
		const auto msgs = recieved.get(Msg::type<Data>());
		std::vector<Data> msgs1;
		msgs1.reserve(msgs.size());
		for (const auto& msg : msgs)
			msgs1.emplace_back(std::move((Data&)*msg.payload()));
		return msgs1;
	}
};