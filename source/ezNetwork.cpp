#include "ezNetwork.h"
#include "ezClient.h"
#include "ezServer.h"


namespace ezNetwork {
	namespace internal{
		Server* _createServer(Address address) {
			return new ezServer(address);
		}

		Client* _createClient(Address server) {
			return new ezClient(server);
		}

		void _deleteServer(Server*server) {
			delete server;
		}

		void _deleteClient(Client*client) {
			delete client;
		}
	}
}

#include "sockets.h"