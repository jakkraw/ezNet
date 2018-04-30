#pragma once
#include <list>
#include <thread>
#include <mutex>
#include <atomic>
#include "serverInfo.h"


using namespace std::chrono_literals;

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")
#include <chrono>


inline std::list<lan::IP> my_ip_list() {
	
	char hostName[80];
	if (gethostname(hostName, sizeof(hostName)) == SOCKET_ERROR) return std::list<lan::IP>();

	const auto host = gethostbyname(hostName);
	if (!host) return std::list<lan::IP>();


	std::list<lan::IP> ipList;
	for (int i = 0; host->h_addr_list[i] != nullptr; ++i)
	{
		in_addr address;
		memcpy(&address, host->h_addr_list[i], sizeof(in_addr));
		ipList.emplace_back(inet_ntoa(address));
	}

	return ipList;
}

using BroadcastIP = lan::IP;

inline std::list<BroadcastIP> possibleBroadcasts(const lan::IP& ip) {
	std::list<BroadcastIP> broadcasts;
	in_addr address; 
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

struct ServerFinder {

	std::list<lan::ServerInfo> servers;
	SOCKET socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	std::thread searcher{&ServerFinder::searchThread, this}, reciever{&ServerFinder::infoThread, this};
	
	~ServerFinder() {
		running = false;
		searcher.join();
		reciever.join();
	}

	std::atomic<bool> running = true;

	void searchThread() {
		bool l = true;
		auto err = setsockopt(socket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char *>(&l), sizeof(l));
		
		if (err) {}
		while (running)
		{
			std::list<lan::IP> ips;

			for (auto&& ip : my_ip_list()) ips.splice(ips.end(), possibleBroadcasts(ip));

			for(auto& ip : ips)
			{
				sockaddr_in broadcast = sockaddr_in();
				broadcast.sin_family = AF_INET;
				broadcast.sin_port = htons(19246);
				broadcast.sin_addr.S_un.S_addr = { inet_addr(ip.c_str()) };

				auto res = sendto(socket, nullptr, 0, 0, (SOCKADDR *)& broadcast, sizeof(broadcast));
				if (res == SOCKET_ERROR) printError();
			}
			std::this_thread::sleep_for(2s);
		}

	}

	void printError() {
		wchar_t* s = nullptr;
		FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr, WSAGetLastError(),
			MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
			(LPWSTR)&s, 0, nullptr);
		fprintf(stderr, "%S\n", s);
		LocalFree(s);
	}

	void infoThread() {
		
		sockaddr_in sender;
		int size = sizeof(sender);

		while (running)
		{

			lan::Port port;
			auto res = recvfrom(socket, (char*)&port, sizeof(port), 0, (SOCKADDR *)& sender, &size);
			if (res < (int)sizeof(port)) { continue; }

			lan::Address a{inet_ntoa(sender.sin_addr), port};

			std::lock_guard<std::mutex> lock(m);
			servers.emplace_back(a);
			servers.unique([](lan::ServerInfo& first, lan::ServerInfo& second)
					{
						return first.address.ip == second.address.ip &&
								first.address.port == second.address.port;
					});
		}
	}

	std::mutex m;

	std::list<lan::ServerInfo> getAllFound() {
		std::lock_guard<std::mutex> lock(m);
		return servers;
	}

};


