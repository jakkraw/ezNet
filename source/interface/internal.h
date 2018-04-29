#pragma once
#include <vector>
#include <typeinfo>

namespace ezm
{
	namespace internal{

		using ID = size_t;
		using Size = size_t;
		using DataPtr = const char*;

		struct IVector {
			virtual void reserve(Size) = 0;
			virtual void emplace_back(DataPtr) = 0;
		};

		template<typename Data>
		struct Vector : IVector {
			std::vector<Data>& vector;
			Vector(std::vector<Data>& vector) : vector(vector) {}
			void reserve(Size size) override { vector.reserve(size); }
			void emplace_back(DataPtr ptr) override { vector.emplace_back((Data&)*ptr); }
		};

		template<typename Msg>
		static constexpr ID id() { return typeid(Msg).hash_code(); }
		template<typename Msg>
		static constexpr Size size() { return sizeof(Msg); }

		struct Server {

			template<typename Msg>
			void send(const Msg& msg) { 
				_send(size<Msg>(), id<Msg>(), &msg); 
			}

			template<typename Msg>
			std::vector<Msg> recieve() { 
				std::vector<Msg> msgs;
				_recieve(id<Msg>(), Vector<Msg>(msgs));
				return msgs; 
			}

		private:
			virtual void _recieve(const ID&, IVector&) = 0;
			virtual void _send(const Size&, const ID&, DataPtr) = 0;
		};
		extern __declspec(dllexport) Server* _createServer(unsigned port);
		extern __declspec(dllexport) void _deleteServer(Server*);

		struct Client {

			template<typename Msg>
			void send(const Msg& msg) {
				_send(size<Msg>(), id<Msg>(), &msg);
			}

			template<typename Msg>
			std::vector<Msg> recieve() {
				std::vector<Msg> msgs;
				_recieve(id<Msg>(), Vector<Msg>(msgs));
				return msgs;
			}

		private:
			virtual void _recieve(const ID&, IVector&) = 0;
			virtual void _send(const Size&, const ID&, DataPtr) = 0;
		};
		extern __declspec(dllexport) Client* _createClient(unsigned port);
		extern __declspec(dllexport) void _deleteClient(Client*);
	}
}