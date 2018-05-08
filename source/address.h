#pragma once
#include <string>
#include <utility>

using Port = unsigned short;
using IP = std::string;

struct Address {
	IP ip;
	Port port;

	Address(IP ip, const Port& port) : ip(std::move(ip)), port(port) {}
	Address(const Address&) = default;
	Address(Address&& a) = default;

	friend bool operator==(const Address& lhs, const Address& rhs) { return lhs.ip == rhs.ip && lhs.port == rhs.port; }
	friend bool operator!=(const Address& lhs, const Address& rhs) { return !(lhs == rhs); }
};

namespace std {
	template <> struct hash<Address> {
		size_t operator()(const Address& a) const {
			const auto h1 = std::hash<IP>{}(a.ip);
			const auto h2 = std::hash<Port>{}(a.port);
			return h1 ^ (h2 << 1);
		}
	};
}