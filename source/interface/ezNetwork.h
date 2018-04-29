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
		Server(Port listen = DefaultPort){
			_server.reset(internal::_createServer(listen));
		}

		template<typename Msg>
		void send(const Msg& msg) { _server->send(msg); }

		template<typename Msg>
		std::vector<Msg> recieve() { return _server->recieve<Msg>(); }

	private:
		using ServerPtr = std::unique_ptr<internal::Server, void(*)(internal::Server*)>;
		ServerPtr _server = { nullptr, internal::_deleteServer };
	};

	struct Client
	{
		Client(Port search = DefaultPort){
			_client.reset(internal::_createClient(search));
		}

		template<typename Msg>
		void send(const Msg& msg) { _client->send(msg); }

		template<typename Msg>
		std::vector<Msg> recieve() { return _client->recieve<Msg>(); }

	private:
		using ClientPtr = std::unique_ptr<internal::Client, void(*)(internal::Client*)>;
		ClientPtr _client = { nullptr, internal::_deleteClient };
	};
}
