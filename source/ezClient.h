#pragma once
#include "interface/internal.h"
#include "interface/types.h"
#include "Connectione.h"
#include "interface/ezNetwork.h"
using namespace lan;
using namespace internal;

struct EzClient : lan::internal::Client {

	MsgQueue toSend, recieved;
	::Connectione connection;

	static SOCKET createClientSocket(const Address& address)
	{
		auto socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		SOCKADDR_IN addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(address.ip.c_str());
		addr.sin_port = htons(address.port);
		auto res = ::connect(socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
		if (res < 0) return -1;
		return socket;
	}

	EzClient(const Address& address)
		: connection(createClientSocket(address), recieved, toSend)
	{}

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