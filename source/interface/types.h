#pragma once

namespace ezm
{
	using Port = unsigned;
	using IP = const char*;

	struct Address {
		const IP ip;
		const Port port;
		Address(const IP& ip, const Port& port) :ip(ip), port(port) {}
	};

}