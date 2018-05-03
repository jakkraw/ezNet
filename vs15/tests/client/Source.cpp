#include "../messages.h"
#include <thread>
#include <chrono>
#include <conio.h>
#include <atomic>

#pragma comment(lib,"ezNetwork.lib")

#include <thread>
#include "../../../source/Connection.h"
#include "../../../source/serverFinder.h"
std::atomic<bool> active = true;

void print_thread(Connection* server)
{
	while (active) {
		for (auto& g : server->recieve<Greet>())
			g.print();

		for (auto& g : server->recieve<Goodbye>())
			g.print();

		using namespace std::chrono_literals;
		std::this_thread::sleep_for(500ms);
	}
}

int main() {
while(true)
{
	ServerFinder finder;
	Connection* server = nullptr;
	printf("SeachringForServer\n");
	while (true) {
		auto servers = finder.servers();
		if (!servers.empty())
			if (server = new Connection(servers.front())) break;
	}

	printf("ServerFound\n");
	active = true;
	std::thread t(&print_thread, server);

	while (true) {
		switch (_getch())
		{
		case '1':
			server->send(Greet());
			break;
		case'2':
			server->send(Goodbye("pozdrowionka"));
			break;
		case'3':
			server->send(Goodbye("nie pozdrawiam"));
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
	delete server;
}
	
}