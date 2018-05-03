#pragma once
#include <list>
#include <thread>
#include <mutex>
#include <atomic>
#include "socket.h"
#include "address.h"

using namespace std::chrono_literals;

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")
#include <chrono>

struct WinSockLifetime {
	WinSockLifetime() { if (WSAStartup(MAKEWORD(2, 2), &WSADATA())) throw; }
	~WinSockLifetime() { WSACleanup(); }
};

extern const WinSockLifetime lifetime = WinSockLifetime();

const Port discoverPort = 19246;
namespace Msgs {
	struct ServerLocation { Port port; };
	struct ServerSearch {};
}


struct Broadcaster {
	std::atomic<bool> running = true;
	const Port port;
	Socket s{ Socket::Type::UDP };
	std::thread searcher{ [this]()
	{
		this->s.setBroadcast(true);
		this->s.setReusable(true);
		this->s.bind(discoverPort);

		while (this->running) {
			const auto msg = this->s.recieveAny<Msgs::ServerSearch>();
			if (msg.valid) this->s.sendTo(msg.from, Msgs::ServerLocation{ this->port });
		}
	} };

	Broadcaster(const Port& port) : port(port) {}

	~Broadcaster() {
		running = false;
		s.shutdown();
		s.close();
		searcher.join();
	}
};

struct ServerFinder {
	std::mutex m;
	std::list<Address> _servers;
	Socket socket{ Socket::Type::UDP };
	std::thread searcher, reciever;
	std::atomic<bool> running = true;

	ServerFinder() {
		socket.setBroadcast(true);
		socket.bind();
		searcher = std::thread{[this]() {
			while (running)
			{
				for (auto&& ip : my_ip_list())
					for (auto&& broadcast : possibleBroadcasts(ip))
						socket.sendTo({ broadcast, discoverPort }, Msgs::ServerSearch());

				std::this_thread::sleep_for(2s);
			}
		} };
		reciever = std::thread{ [this](){
				while (running)
				{
					auto msg = socket.recieveAny<Msgs::ServerLocation>();
					if (msg.valid) {
						std::lock_guard<std::mutex> lock(m);
						_servers.emplace_back(msg.from.ip, msg.data.port);
						_servers.unique([](Address& first, Address& second)
						{
							return first.ip == second.ip &&
								first.port == second.port;
						});
					}
				}
			} };
	}

	~ServerFinder() {
		running = false;
		socket.shutdown();
		socket.close();
		searcher.join();
		reciever.join();
	}

	std::list<Address> servers() {
		std::lock_guard<std::mutex> lock(m);
		return _servers;
	}

	static std::list<IP> my_ip_list() {
		char hostName[80];
		if (gethostname(hostName, sizeof(hostName)) == SOCKET_ERROR) return std::list<IP>();

		const auto host = gethostbyname(hostName);
		if (!host) return std::list<IP>();


		std::list<IP> ipList;
		for (int i = 0; host->h_addr_list[i] != nullptr; ++i)
		{
			in_addr address;
			memcpy(&address, host->h_addr_list[i], sizeof(in_addr));
			ipList.emplace_back(inet_ntoa(address));
		}

		return ipList;
	}

	static std::list<IP> possibleBroadcasts(const IP& ip) {
		std::list<IP> broadcasts;
		in_addr address{};
		address.s_addr = inet_addr(ip.c_str());

		broadcasts.emplace_back(inet_ntoa(address));
		address.S_un.S_un_b.s_b4 = 255;
		broadcasts.emplace_back(inet_ntoa(address));
		address.S_un.S_un_b.s_b3 = 255;
		broadcasts.emplace_back(inet_ntoa(address));
		address.S_un.S_un_b.s_b2 = 255;
		broadcasts.emplace_back(inet_ntoa(address));

		return broadcasts;
	}
};


