#include "interface/internal.h"
#include "ezClient.h"
#include "ezServer.h"


namespace ezm {
	namespace internal{
		Server* _createServer(Port listenPort) {
			return new ezServer(listenPort);
		}

		Client* _createClient(Port searchPort) {
			return new ezClient(searchPort);
		}

		void _deleteServer(Server*server) {
			delete server;
		}

		void _deleteClient(Client*client) {
			delete client;
		}
	}
}

