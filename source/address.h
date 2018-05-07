#pragma once
#include <string>

using Port = unsigned short;
using IP = std::string;

struct Address {
	IP ip;
	Port port;

	Address(const IP& ip, const Port& port) :ip(ip), port(port) {}

	Address(const Address& a) : ip(a.ip), port(a.port){};

	Address(Address&& a) noexcept
		:ip(std::move(a.ip)), port(std::move(a.port)) {}

	friend bool operator==(const Address& lhs, const Address& rhs) {
		return lhs.ip == rhs.ip && lhs.port == rhs.port;
	}

	friend bool operator!=(const Address& lhs, const Address& rhs) {
		return !(lhs == rhs);
	}

	std::size_t operator()(Address const& s) const noexcept {
		const auto h1 = std::hash<IP>{}(ip);
		const auto h2 = std::hash<Port>{}(port);
		return h1 ^ (h2 << 1);
	}

};

namespace std {
	template<>
	struct hash<Address> {
		size_t operator()(const Address& a) const {
			const auto h1 = std::hash<IP>{}(a.ip);
			const auto h2 = std::hash<Port>{}(a.port);
			return h1 ^ (h2 << 1);
		}
	};
}
