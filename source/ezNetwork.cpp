#include "ezNetwork.h"

using namespace ezNetwork;
using namespace ezNetwork::internal;

struct MsgInterface : public virtual DataFlow {


	struct Msg {
		ID id;
		Size size;
		bool Msg::operator == (const Msg &b) const {
			return id == b.id;
		}
	};
	struct MsgHasher
	{
		std::size_t operator()(const Msg& k) const
		{
			using std::size_t;
			using std::hash;
			using std::string;
			return hash<ID>()(k.id);
		}
	};

	typedef char Byte;
	typedef std::vector<Byte> Data;
	typedef std::unordered_map<Msg, std::vector<Data>, MsgHasher> Messages;
	typedef std::unordered_map < Connection::ID,
		std::unique_ptr < Connection >> Connections;
	Messages toSend, recieved;
	Connections clients;


	virtual void send_raw(ID id, Size size, const void* data, Target = TargetAll) {
		auto& messages = toSend;
		messages[{id, size}].emplace_back((Byte*)data, (Byte*)data + size);
	}

	virtual void recieve_raw(ID id, VectorRawRecive& target) {
		auto& messages = toSend;

		auto& it = messages.find({ id,0 });
		if (it == messages.end()) return;

		auto& source = it->second;
		target.reserve(source.size());
		for (const auto& msg : source)
			target.add((void*)msg.data());

		source.clear();
	}
};

struct ServerImpl : private MsgInterface, public Server {

};

struct ClientImpl : private MsgInterface, public Client {

};

Server* ezNetwork::internal::_createServer(Address address) {
	return new ServerImpl;
}

Client* ezNetwork::internal::_createClient(Address server) {
	return new ClientImpl;
}

void ezNetwork::internal::_deleteServer(Server*server) {
	delete server;
}

void ezNetwork::internal::_deleteClient(Client*client) {
	delete client;
}
