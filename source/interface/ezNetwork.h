#pragma once
#include <memory>
#include "types.h"
#include "internal.h"

namespace ezm
{
	constexpr Port DefaultPort{ 19245 };
	constexpr IP Localhost{ "127.0.0.1" };
	const Address DefaultAddress{ Localhost, DefaultPort };



	struct Server
	{
		Server(Port port = DefaultPort) 
			: server(internal::_createServer(port)) {}

		template<typename Msg>
		void send(const Msg& msg) { server->send(msg); }

		template<typename Msg>
		std::vector<Msg> recieve() { return server->recieve<Msg>(); }

	private:
		using ServerPtr = std::unique_ptr<internal::Server, void(*)(internal::Server*)>;
		ServerPtr server = { nullptr, internal::_deleteServer };
	};



	struct Client
	{
		Client(Port port = DefaultPort)
			: client(internal::_createClient(port)) {}

		template<typename Msg>
		void send(const Msg& msg) { client->send(msg); }

		template<typename Msg>
		std::vector<Msg> recieve() { return client->recieve<Msg>(); }

	private:
		using ClientPtr = std::unique_ptr<internal::Client, void(*)(internal::Client*)>;
		ClientPtr client = { nullptr, internal::_deleteClient };
	};
}
