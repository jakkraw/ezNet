#include "../messages.h"
#include "../../../source/interface/ezNetwork.h"
#include <thread>
#include <chrono>
#include <conio.h>
#pragma comment(lib,"ezNetwork.lib")
using namespace lan;

#include <thread>

void print_thread(Connection* server)
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
	ConnectionPtr server;
	while(true)
	{
		auto servers = finder.getAllFound();
		if(!servers.empty()){
			server = servers.front().connect();
			if (server) break;
		}

	}


	std::thread t(&print_thread, server.get());

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

	//t.join();
	return 0;
}