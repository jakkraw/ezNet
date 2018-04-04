#pragma once
#include <memory>
#include <list>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <functional>
#include <iterator>

namespace ezNetwork {

	struct Address {
		typedef unsigned int Port;
		typedef const char* IP;
		const IP ip;
		const Port port;
		Address(IP ip, Port port) :ip(ip), port(port) {}
	} const DefaultAddress = Address("localhost", 19245);

	struct Connection {
		typedef int ID;
		const ID id;
		const Address address;
	};

	struct Target {
		bool(*filter)(const Connection&);
	} const TargetAll = { [](const Connection&) { return true; } };


	namespace internal{
		struct DataFlow {
			struct VectorRawRecive {
				virtual void reserve(size_t) = 0;
				virtual void add(void* ptr) = 0;
			};

			template<typename Data>
			struct VectorWrapper : public VectorRawRecive {
				std::vector<Data>& vector;
				VectorWrapper(std::vector<Data>& vector) : vector(vector) {}
				virtual void reserve(size_t size) override { vector.reserve(size); }
				virtual void add(void* ptr) { vector.push_back(*(Data*)ptr); }
			};

			typedef int ID;
			typedef size_t Size;
			virtual void send_raw(ID, Size, const void* data, Target = TargetAll) = 0;
			virtual void recieve_raw(ID, VectorRawRecive&) = 0;
			virtual ~DataFlow() = default;

			template<typename Data>
			static constexpr DataFlow::ID id() { return typeid(Data).hash_code(); }
			template<typename Data>
			static constexpr DataFlow::Size size() { return sizeof(Data); }

			template<typename Data>
			std::vector<Data> recieve() {
				std::vector<Data> vector;
				recieve_raw(id<Data>(), VectorWrapper<Data>(vector));
				return vector;
			}
		};
	}

	struct Server : private virtual internal::DataFlow {
		template<typename Data>
		void send(const Data& data, Target target = TargetAll) {
			send_raw(id<Data>(), size<Data>(), &data, target);
		}

		template<typename Data>
		std::vector<Data> recieve() {
			return DataFlow::recieve<Data>();
		}
	};

	struct Client : private virtual internal::DataFlow {
		template<typename Data>
		void send(const Data& data) {
			send_raw(id<Data>(),size<Data>(), &data);
		}

		template<typename Data>
		std::vector<Data> recieve() {
			return DataFlow::recieve<Data>();
		}
	};

	namespace internal{
		extern Server* _createServer(Address address = DefaultAddress);
		extern void _deleteServer(Server*);

		extern Client* _createClient(Address server = DefaultAddress);
		extern void _deleteClient(Client*);
	}

	inline std::unique_ptr<Server, void(*)(Server*)> createServer(Address address = DefaultAddress) {
		return std::unique_ptr<Server, void(*)(Server*)>(internal::_createServer(address), internal::_deleteServer);
	}

	inline std::unique_ptr<Client, void(*)(Client*)> createClient(Address server = DefaultAddress) {
		return std::unique_ptr<Client, void(*)(Client*)>(internal::_createClient(server), internal::_deleteClient);
	}

}