#pragma once
#include <memory>
#include <unordered_map>
#include <functional>

namespace ezNetwork {

	struct Address {
		using Port = unsigned int;
		using IP = const char*;
		const IP ip;
		const Port port;
		Address(IP ip, Port port) :ip(ip), port(port) {}
	} const DefaultAddress = Address("127.0.0.1", 19245);


	namespace internal{
		struct DataFlow {
			using ID = size_t;
			using Size = size_t;
			template<typename Data>
			static constexpr ID id() { return typeid(Data).hash_code(); }
			template<typename Data>
			static constexpr Size size() { return sizeof(Data); }

			struct VectorRawRecive {
				virtual void reserve(size_t) = 0;
				virtual void add(void*) = 0;
			};

			template<typename Data>
			struct VectorWrapper : public VectorRawRecive {
				std::vector<Data>& vector;
				VectorWrapper(std::vector<Data>& vector) : vector(vector) {}
				void reserve(size_t size) override { vector.reserve(size); }
				void add(void* ptr) override { vector.push_back(*(Data*)ptr); }
			};

			virtual void send_raw(ID, Size, const void* data) = 0;
			virtual void recieve_raw(ID, VectorRawRecive&) = 0;
			virtual ~DataFlow() = default;

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
		void send(const Data& data) {
			send_raw(id<Data>(), size<Data>(), &data);
		}

		template<typename Data>
		std::vector<Data> recieve() {
			return DataFlow::recieve<Data>();
		}
	};

	struct Client : private virtual internal::DataFlow {
		template<typename Data>
		void send(const Data& data) {
			send_raw(id<Data>(), size<Data>(), &data);
		}

		template<typename Data>
		std::vector<Data> recieve() {
			return DataFlow::recieve<Data>();
		}
	};

	namespace internal{
		extern __declspec(dllexport) Server* _createServer(Address address = DefaultAddress);
		extern __declspec(dllexport) void _deleteServer(Server*);
		extern __declspec(dllexport) Client* _createClient(Address server = DefaultAddress);
		extern __declspec(dllexport) void _deleteClient(Client*);
	}

	inline std::unique_ptr<Server, void(*)(Server*)> createServer(Address address = DefaultAddress) {
		return std::unique_ptr<Server, void(*)(Server*)>(internal::_createServer(address), internal::_deleteServer);
	}

	inline std::unique_ptr<Client, void(*)(Client*)> createClient(Address server = DefaultAddress) {
		return std::unique_ptr<Client, void(*)(Client*)>(internal::_createClient(server), internal::_deleteClient);
	}

}
