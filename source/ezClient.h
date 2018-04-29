#pragma once
#include "interface/internal.h"
#include "interface/types.h"
#include "sockets.h"
using namespace ezm;
using namespace internal;


struct ezClient : Client {


	Connection connection;

	static SOCKET createClientSocket(Address server)
	{
		SOCKET socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		SOCKADDR_IN addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(server.ip);
		addr.sin_port = htons(server.port);
		::connect(socket, (sockaddr*)&addr, sizeof(addr));
		return socket;
	}

	ezClient(Address address) : connection(createClientSocket(address))
	{
		
	}


	MsgQueue toSend, recieved;

	void _send(const Size&, const ID&, DataPtr) override {
		//toSend.add(msg);
	}

	void _recieve(const ID& id, IVector& target) override {
		const auto data = recieved.get(id);

		target.reserve(data.size());
		for (const auto& msg : data)
			target.emplace_back(msg.data() + sizeof(Msg::Header));
	}


};