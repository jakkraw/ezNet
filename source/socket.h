#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <utility>
#pragma comment(lib, "Ws2_32.lib")

struct WinSockLifetime {
	WinSockLifetime() { if (WSAStartup(MAKEWORD(2, 2), &WSADATA())) throw; }
	~WinSockLifetime() { WSACleanup(); }
};

extern const WinSockLifetime lifetime = WinSockLifetime();

#include "message.h"
#include "address.h"

struct Socket {
	SOCKET socket;

	Socket(const SOCKET& socket)
		: socket(socket) {}

	Socket(Socket&& socket)
		: socket(socket.socket) { socket.socket = INVALID_SOCKET; }

	void setInvalid() {
		printError();
		close();
		socket = INVALID_SOCKET;
	}

	void close() {
		if (!isValid()) return;
		const auto result = closesocket(socket);
		if (result == SOCKET_ERROR)
		{
			socket = INVALID_SOCKET;
			printError();
		}
	}

	static void printError() {
		wchar_t* error = nullptr;
		FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		               nullptr, WSAGetLastError(),
		               MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
		               LPWSTR(&error), 0, nullptr);
		fprintf(stderr, "%S\n", error);
		LocalFree(error);
	}

	void setBroadcast(bool enable) {
		if (!isValid()) return;
		const auto result = setsockopt(socket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char *>(&enable), sizeof(enable));
		if (result == SOCKET_ERROR) setInvalid();
	}

	void setReusable(bool enable) {
		if (!isValid()) return;
		const auto result = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char *>(&enable), sizeof(enable));
		if (result == SOCKET_ERROR) setInvalid();
	}

	void shutdown() {
		if (!isValid()) return;
		if (::shutdown(socket, SD_BOTH) == SOCKET_ERROR) setInvalid();
	}

	bool isValid() const { return socket != INVALID_SOCKET; }

	template <class Data> struct Recieved {
		Recieved() {}

		Recieved(Address from, const Data& data)
			: valid(true),
			  from(std::move(from)),
			  data(data) {}

		bool valid{false};
		Address from{"0.0.0.0", 0};

		union {
			Data data;
			bool _ = false;
		};
	};

	template <class Data> Recieved<Data> recieveAny() {

		if (!isValid()) return {};
		sockaddr_in sender;
		int senderSize = sizeof(sender);

		Msg msg(sizeof(Data));
		auto result = recvfrom(socket, msg, msg.size(), 0,
		                       reinterpret_cast<sockaddr*>(&sender), &senderSize);

		if (result == SOCKET_ERROR) goto severeError;
		if (result != static_cast<int>(msg.size())) goto badMsg;
		if (typeid(Data).hash_code() != msg.header().type) goto badMsg;

		return {{inet_ntoa(sender.sin_addr), ntohs(sender.sin_port)}, msg.payloadAs<Data>()};
	severeError: printf("udpRecieve:"), printError();
	badMsg: return {};
	}

	template <class Data> bool sendTo(const Address& address, const Data& data) {
		if (!isValid()) return false;
		sockaddr_in reciever = sockaddr_in();
		reciever.sin_family = AF_INET;
		reciever.sin_port = htons(address.port);
		reciever.sin_addr.s_addr = inet_addr(address.ip.c_str());

		Msg msg{Msg::toMsg(data)};
		const auto result = sendto(socket, msg, msg.size(), 0, reinterpret_cast<const sockaddr*>(&reciever),
		                           sizeof(reciever));
		if (result == SOCKET_ERROR) printf("udpSend:"), printError();
		return result > SOCKET_ERROR;
	}

	bool bind(const Port& port = 0, const IP& ip = "0.0.0.0") {
		if (!isValid()) return false;
		sockaddr_in addr = sockaddr_in();
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = inet_addr(ip.c_str());

		const auto result = ::bind(socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
		if (result == SOCKET_ERROR) setInvalid();
		return result > SOCKET_ERROR;
	}

	bool listen() {
		if (!isValid()) return false;
		const auto result = ::listen(socket, SOMAXCONN);
		if (result == SOCKET_ERROR) setInvalid();
		return result > SOCKET_ERROR;
	}

	Address address() {
		if (!isValid()) return {"0.0.0.0", 0};
		sockaddr_in addr;
		int size = sizeof(addr);
		const auto result = getsockname(socket, reinterpret_cast<sockaddr*>(&addr), &size);
		if (result == SOCKET_ERROR) setInvalid();
		return {inet_ntoa(addr.sin_addr), ntohs(addr.sin_port)};
	}

	bool connect(const Address& address) {
		if (!isValid()) return false;
		SOCKADDR_IN addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(address.ip.c_str());
		addr.sin_port = htons(address.port);

		const auto result = ::connect(socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
		if (result == SOCKET_ERROR) setInvalid();
		return result > SOCKET_ERROR;
	}

	std::pair<Socket, Address> accept() {
		if (!isValid()) return {Socket{INVALID_SOCKET}, Address{"0.0.0.0", 0}};
		sockaddr_in addr = sockaddr_in();
		int size = sizeof(addr);
		const auto client = ::accept(socket, reinterpret_cast<sockaddr*>(&addr), &size);
		if (client == INVALID_SOCKET) setInvalid();

		return {Socket{client}, Address{inet_ntoa(addr.sin_addr), ntohs(addr.sin_port)}};
	}

	bool send(const Msg& msg) {
		if (!isValid()) return false;
		const auto result = ::send(socket, msg, msg.size(), 0);
		if (result == SOCKET_ERROR) setInvalid();
		return result > SOCKET_ERROR;
	}

	struct Package {
		Package(Msg&& msg)
			: valid(true),
			  msg(std::move(msg)) {}

		Package(Package&& data) noexcept
			: valid(data.valid),
			  msg(std::move(data.msg)) {}

		Package() {}
		~Package() {}

		const bool valid{false};

		union {
			Msg msg;
			const char _ = 0;
		};
	};

	Package recieve() {
		if (!isValid()) return {};
		Msg::Size size;

		const auto result = ::recv(socket, reinterpret_cast<char*>(&size), sizeof(size), MSG_PEEK);
		if (result == SOCKET_ERROR) goto severeError;
		if (result != sizeof(Msg::Size)) goto badMsg;

		{
			Msg msg(size);
			const auto result = ::recv(socket, msg, msg.size(), MSG_WAITALL);
			if (result == SOCKET_ERROR) goto severeError;
			if (result != static_cast<int>(msg.size())) goto badMsg;

			msg.setSender(socket);
			return std::move(msg);
		}

	severeError: setInvalid();
	badMsg: return {};
	}

	~Socket() { close(); }

};

struct TcpSocket : Socket {
	TcpSocket()
		: Socket(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) {}
};

struct UdpSocket : Socket {
	UdpSocket()
		: Socket(::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) {}
};
