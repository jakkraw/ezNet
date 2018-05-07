#pragma once
#include <list>
#include <thread>
#include <mutex>
#include <atomic>
#include "socket.h"
#include "address.h"
#include <unordered_set>
#include <chrono>

using namespace std::chrono_literals;

const Port discoverPort = 19246;
namespace Msgs {
	struct ServerLocation { Port port; };
	struct ServerSearch {};
}


struct Broadcaster {
	std::atomic<bool> running = true;
	const Port port;
	UdpSocket socket;
	std::thread searcher{ [this]()
	{
		socket.setBroadcast(true);
		socket.setReusable(true);
		socket.bind(discoverPort);

		while (running) {
			auto msg = socket.recieveAny<Msgs::ServerSearch>();
			if (msg.valid) socket.sendTo(msg.from, Msgs::ServerLocation{ port });
		}
	} };

	Broadcaster(const Port& port) : port(port) {}

	~Broadcaster() {
		running = false;
		socket.shutdown();
		socket.close();
		searcher.join();
	}
};


template<typename Data>
class ConcurrentUnorderedSet {
	std::mutex mutex;
	std::unordered_set<Data> set;


	public:
	void insert(Data&& address) {
		std::lock_guard<std::mutex> lock(mutex);
		set.insert(std::move(address));
	}

	std::unordered_set<Data> get() {
		std::lock_guard<std::mutex> lock(mutex);
		return set;
	}
};

struct ServerFinder {

	ConcurrentUnorderedSet<Address> _servers;
	UdpSocket socket;
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
					const auto msg = socket.recieveAny<Msgs::ServerLocation>();
					if (msg.valid) _servers.insert({ msg.from.ip, msg.data.port });
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

	std::unordered_set<Address> servers() {
		return _servers.get();
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


