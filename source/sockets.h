#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")



struct MsgWrapper
{
	using Byte = char;
	using ID = internal::DataFlow::ID;
	using Size = internal::DataFlow::Size;
	Size size;
	ID id;
	std::vector<Byte> data;

	size_t toBuffer(Byte* buffer, size_t size)
	{
		memcpy_s(buffer, size, &this->size, sizeof(this->size));
		buffer += sizeof(this->size);
		memcpy_s(buffer, size, &id, sizeof(id));
		buffer += sizeof(id);
		memcpy_s(buffer, size, data.data(), data.size());
		return sizeof(this->size) + sizeof(id) + data.size();
	}

};

struct Connection {
	SOCKET socket;
	Connection(SOCKET socket) : socket(socket) {}

	~Connection()
	{
		shutdown(socket, SD_BOTH);
		closesocket(socket);
	}

	void send(const MsgWrapper& msg)
	{
		
	}

	void recieve()

};

SOCKET createClientSocket(Address server)
{
	SOCKET socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(server.ip);
	addr.sin_port = htons(server.port);
	::connect(socket, (sockaddr*)&addr, sizeof(addr));
}


#include <list>
std::list<std::string> myIPList()
{
	std::list<std::string> ipList;
	char hostName[80];
	if (gethostname(hostName, sizeof(hostName)) == SOCKET_ERROR) return ipList;

	const auto host = gethostbyname(hostName);
	if (!host) return ipList;

	for (int i = 0; host->h_addr_list[i] != 0; ++i) {
		in_addr address;
		memcpy(&address, host->h_addr_list[i], sizeof(in_addr));
		ipList.emplace_back(inet_ntoa(address));
	}

	return ipList;
}

struct WinSockLifetime {
	WinSockLifetime() {
		if (WSAStartup(MAKEWORD(2, 2), &WSADATA())) throw;
	}
	~WinSockLifetime() { WSACleanup(); }
};
extern const WinSockLifetime lifetime = WinSockLifetime();



