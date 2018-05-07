#include "../messages.h"
#include <thread>
#include <chrono>
#include <conio.h>
#include <atomic>
#include <memory>

//#pragma comment(lib,"ezNetwork.lib")

#include <thread>
#include "../../../source/connection.h"
#include "../../../source/serverFinder.h"


void printer(Connection& server, std::atomic_bool& active)
{
	while (active) {
		for (auto& g : server.recieve<Greet>())
			g.print();

		for (auto& g : server.recieve<Goodbye>())
			g.print();

		using namespace std::chrono_literals;
		std::this_thread::sleep_for(100ms);
	}
}

Address findServer() {
	ServerFinder finder;
	printf("SeachringForServer\n");
	while (true) {
		const auto servers = finder.servers();
		if (!servers.empty())
			return *servers.begin();
	}
}

int main() {
	std::atomic<bool> active = true;
	std::unique_ptr<Connection> server = std::make_unique<Connection>(findServer());
	printf("ServerFound\n");

	std::thread t(&printer, std::ref(*server), std::ref(active));

	while (true) {
		switch (_getch())
		{
		case '1':
			server->send(Greet());
			break;
		case'2':
			server->send(Goodbye("Goodbye..."));
			break;
		case'3':
			server->send(Goodbye("Bye."));
			break;
		case'q':
			active = false;
			t.join();
			return 0;
		}
		if (!server->isValid()) break;
	}

	printf("ServerFoundInvalid\n");
	active = false;
	t.join();
}