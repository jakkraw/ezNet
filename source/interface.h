#pragma once
#include <unordered_set>
#include "address.h"
#include "msg.h"
#include <memory>

//namespace itf {
//	
//	struct MsgExchange {
//		enum Transfer { Reliable , Fast };
//		enum Type { Direct, Broadcast };
//
//		using MsgPtr = std::unique_ptr<Msg>;
//		
//		virtual void send(MsgPtr, Transfer = Reliable, Type = Direct) = 0;
//		virtual std::list<MsgPtr> get(Msg::Type, Transfer = Reliable, Type = Direct) = 0;
//
//	};
//
//	struct Connection {
//		using ID = size_t;
//
//
//		virtual ID id() const = 0;
//
//
//	};
//
//	struct Server{
//		struct Connections {
//			
//		};
//
//		struct Broadcaster {
//			
//		};
//
//		virtual Connection::ID generate() = 0;
//
//	};
//
//
//	struct Finder {
//		virtual std::unordered_set<Address> servers() = 0;
//	};
//
//	struct Client {
//		
//	};
//}
