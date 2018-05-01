#pragma once
#include "msgQueue.h"
#include "socket.h"
#include "address.h"
#include <atomic>


struct EzClient {
	MsgQueue toSend, recieved;
	Socket socket{ Socket::Type::TCP };
	std::thread sender, reciever;
	std::atomic<bool> active{true};

	explicit EzClient(const Address& address) {
		socket.connect(address);

		sender = std::thread{ [this]()
			{
				while(active)
					for (auto&& key : toSend.getAny())
						for (auto&& msg : key.second)
							socket.send(msg);

			} };

		reciever = std::thread{ [this]()
		{
			while (active) {
				auto package = socket.recieve();
				if (package.valid)
					recieved.add(std::move(package.msg));
			}

		}};

	}

	~EzClient() {
		active = false;
		sender.join();
		reciever.join();
	}

	template<typename Data>
	void send(const Data& data) {
		toSend.add({ Msg::size<Data>(), Msg::type<Data>(), (Msg::Data)&data });
	}

	bool isValid() const { return socket.isValid(); };

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
