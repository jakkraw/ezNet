#include "../messages.h"
#include <thread>
#include <chrono>
#include <conio.h>

#pragma comment(lib,"ezNetwork.lib")

#include <thread>
#include "../../../source/ezClient.h"
#include "../../../source/serverFinder.h"

void print_thread(EzClient* server)
{
	while (server) {
		for (auto& g : server->recieve<Greet>())
			g.print();

		for (auto& g : server->recieve<Goodbye>())
			g.print();

		using namespace std::chrono_literals;
		std::this_thread::sleep_for(500ms);
	}
}

int main() {
	ServerFinder finder;

	EzClient* server;
	while(true){ 
		auto servers = finder.servers();
		if(!servers.empty())
			if (server = new EzClient(servers.front())) break;
	}


	std::thread t(&print_thread, server);

	while (true)
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
		case'q': return 0;
		}
}