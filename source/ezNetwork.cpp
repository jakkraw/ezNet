#include "interface/internal.h"
#include "ezClient.h"
#include "ezServer.h"


namespace lan {
	namespace internal{
		Server* _createServer() {
			return new EzServer();
		}

		Client* _createClient(const Address& address) {
			return new EzClient(address);
		}

		void _deleteServer(Server*server) {
			delete server;
		}

		void _deleteClient(Client*client) {
			delete client;
		}
	}
}

