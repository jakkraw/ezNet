#pragma once
#include "interface/internal.h"
#include "interface/types.h"
#include "connection.h"
using namespace ezm;
using namespace internal;

struct ezClient : ezm::internal::Client {

	MsgQueue toSend, recieved;
	Connection connection;

	static SOCKET createClientSocket(Port serachPort)
	{
		auto socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		SOCKADDR_IN addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		addr.sin_port = htons(serachPort);
		auto res = ::connect(socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
		if (res < 0) return -1;
		return socket;
	}

	ezClient(Port serachPort)
		: connection(createClientSocket(serachPort), recieved, toSend)
	{
		
	}

	void _send(const Size& size, const ID& id, DataPtr data) override {
		toSend.add({size, id, data});
	}

	void _recieve(const ID& id, IVector& target) override {
		const auto msgs = recieved.get(id);
		target.reserve(msgs.size());
		for (const auto& msg : msgs)
			target.emplace_back(msg.payload());
	}

};