#pragma once
#include <string>

using Port = unsigned short;
using IP = std::string;

struct Address {
	const IP ip;
	const Port port;
	Address(const IP& ip, const Port& port) :ip(ip), port(port) {}
};