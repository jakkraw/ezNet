#include "interface/internal.h"
#include "ezClient.h"
#include "ezServer.h"
#include "sockets.h"


namespace ezMultiplayer {
	namespace internal{
		Server* _createServer(unsigned port) {
			return new ezServer(port);
		}

		Client* _createClient(unsigned port) {
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

