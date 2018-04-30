#pragma once
#include "ezNetwork.h"

namespace lan {
	struct Connection
	{
		Connection(const Address& addr) {
			_client.reset(internal::_createClient(addr));
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