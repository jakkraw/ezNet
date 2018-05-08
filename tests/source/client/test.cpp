#include <conio.h>
#include <atomic>
#include <thread>
#include <chrono>
using namespace std::chrono_literals;

#include "../messages.h"
#include "connection.h"
#include "serverFinder.h"

void printer(Connection& server, std::atomic_bool& active) {
	while (active)
	{
		for (auto&& msg : server.recieve<Greet>()) msg.print();

		for (auto&& msg : server.recieve<Text100>()) msg.print();

		if (!server.isValid())
		{
			printf("Server Connection Error\n");
			active = false;
		}

		std::this_thread::sleep_for(0s);
	}
}

Address find_server() {
	ServerFinder finder;
	printf("SeachringForServer\n");
	while (true)
	{
		const auto servers = finder.servers();
		if (!servers.empty()) return printf("ServerFound\n"), *servers.begin();
		std::this_thread::sleep_for(0s);
	}
}

int main() {
	std::atomic_bool active{true};
	Connection server{find_server()};

	std::thread printer_thread(&printer, std::ref(server), std::ref(active));

	auto nr = 0;

	while (active)
		switch (_getch())
		{
		case '1': server.send(Greet{ ++nr });
				break;
			case'2': server.send(Text100("Goodbye..."));
				break;
			case'3': server.send(Text100("Bye."));
				break;
			case'q': active = false;
				break;
		}

	printer_thread.join();
}
