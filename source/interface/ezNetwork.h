#pragma once
#include <memory>
#include "types.h"
#include "internal.h"
#include "ServerFinder.h"
#include "connection.h"

namespace lan
{
	const Port DefaultPort{ 19245 };
	const IP Localhost{ "127.0.0.1" };
	const Address DefaultAddress{ Localhost, DefaultPort };

	struct Server
	{
		Server(){
			_server.reset(internal::_createServer());
		}

		template<typename Msg>
		void send(const Msg& msg) { _server->send(msg); }

		template<typename Msg>
		std::vector<Msg> recieve() { return _server->recieve<Msg>(); }

	private:
		using ServerPtr = std::unique_ptr<internal::Server, void(*)(internal::Server*)>;
		ServerPtr _server = { nullptr, internal::_deleteServer };
	};



	

}



