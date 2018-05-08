#pragma once
#include <list>
#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_set>
#include <chrono>

#include "socket.h"
#include "address.h"

using namespace std::chrono_literals;

const Port DISCOVER_PORT = 19246;

namespace Msgs {
	struct ServerLocation { Port port; };
	struct ServerSearch {};
}

struct Broadcaster {
	explicit Broadcaster(const Port& port) : port(port) {}
	~Broadcaster() {
		running = false;
		socket.shutdown();
		socket.close();
		searcher.join();
	}

	private:
		std::atomic_bool running{ true };
		const Port port;
		UdpSocket socket;
		std::thread searcher{
			[this]()
		{
			socket.setBroadcast(true);
			socket.setReusable(true);
			socket.bind(DISCOVER_PORT);

			while (running)
			{
				const auto msg = socket.recieveAny<Msgs::ServerSearch>();
				if (msg.valid) socket.sendTo(msg.from, Msgs::ServerLocation{ port });
			}
		}
		};
};

template <typename Data> struct ConcurrentUnorderedSet {

	void insert(Data&& address) {
		std::lock_guard<std::mutex> lock(mutex);
		set.insert(std::move(address));
	}

	std::unordered_set<Data> get() {
		std::lock_guard<std::mutex> lock(mutex);
		return set;
	}

	private:
	std::mutex mutex;
	std::unordered_set<Data> set;

};

struct ServerFinder {

	std::unordered_set<Address> servers() { return _servers.get(); }

	ServerFinder() {
		socket.setBroadcast(true);
		socket.bind();
		searcher = std::thread{
			[this]()
		{
			while (running)
			{
				for (auto&& ip : my_ip_list())
					for (auto&& broadcast : possibleBroadcasts(ip)) socket.sendTo({ broadcast, DISCOVER_PORT }, Msgs::ServerSearch());

				std::this_thread::sleep_for(2s);
			}
		}
		};
		reciever = std::thread{
			[this]()
		{
			while (running)
			{
				const auto msg = socket.recieveAny<Msgs::ServerLocation>();
				if (msg.valid) _servers.insert({ msg.from.ip, msg.data.port });
			}
		}
		};
	}
	~ServerFinder() {
		running = false;
		socket.shutdown();
		socket.close();
		searcher.join();
		reciever.join();
	}

	private:
		ConcurrentUnorderedSet<Address> _servers;
		UdpSocket socket;
		std::thread searcher, reciever;
		std::atomic<bool> running = true;

	static std::list<IP> my_ip_list() {
		std::list<IP> ips;

		char hostName[80];
		if (gethostname(hostName, sizeof(hostName)) == SOCKET_ERROR) return ips;

		const auto host = gethostbyname(hostName);
		if (!host) return ips;

		
		for (auto i = 0; host->h_addr_list[i] != nullptr; ++i)
		{
			in_addr address;
			memcpy(&address, host->h_addr_list[i], sizeof(in_addr));
			ips.emplace_back(inet_ntoa(address));
		}

		return ips;
	}

	static std::list<IP> possibleBroadcasts(const IP& ip) {
		in_addr address{};
		address.s_addr = inet_addr(ip.c_str());

		std::list<IP> broadcasts;
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
