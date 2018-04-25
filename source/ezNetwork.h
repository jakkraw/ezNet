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
		Address(const IP& ip, const Port& port) :ip(ip), port(port) {}
	} const DefaultAddress = Address("127.0.0.1", 19245);


	namespace internal{
		struct DataFlow {
			using Byte = char;
			using ID = size_t;
			using Size = size_t;
			template<typename Data>
			static constexpr ID id() { return typeid(Data).hash_code(); }
			template<typename Data>
			static constexpr Size size() { return sizeof(Data); }

			struct VectorI {
				virtual void reserve(Size) = 0;
				virtual void add(const Byte*) = 0;
			};

			template<typename Data>
			struct Vector : VectorI {
				std::vector<Data>& vector;
				Vector(std::vector<Data>& vector) : vector(vector) {}
				void reserve(Size size) override { vector.reserve(size); }
				void add(const Byte* ptr) override { vector.emplace_back(*(Data*)ptr); }
			};

		
			struct Msg{
				Size size;
				ID id;
				const Byte* data;
				Msg(const Size& size, const ID& id, const Byte* data) :
					size(size), id(id), data(data){}
			};


			template<class Data>
			Msg toMsg(const Data& data){
				return{ DataFlow::size<Data>(), DataFlow::id<Data>(), (const Byte*)&data };
			}

			virtual void send_raw(const Msg& msg) = 0;
			virtual void recieve_raw(const ID&, VectorI&) = 0;
			virtual ~DataFlow() = default;

			template<typename Data>
			std::vector<Data> recieve() {
				std::vector<Data> vector;
				recieve_raw(id<Data>(), Vector<Data>(vector));
				return vector;
			}
		};
	}

	struct Server : private virtual internal::DataFlow {
		template<typename Data>
		void send(const Data& data) {
			send_raw(toMsg(data));
		}

		template<typename Data>
		std::vector<Data> recieve() {
			return DataFlow::recieve<Data>();
		}
	};

	struct Client : private virtual internal::DataFlow {
		template<typename Data>
		void send(const Data& data) {
			send_raw(toMsg(data));
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
