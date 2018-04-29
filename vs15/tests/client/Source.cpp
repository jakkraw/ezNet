#include "../messages.h"
#include "../../../source/interface/ezNetwork.h"
#include <thread>
#include <chrono>
#include <conio.h>
#pragma comment(lib,"ezNetwork.lib")
using namespace ezm;

#include <thread>

void print_thread(Client* server)
{
	while (true) {
		for (auto& g : server->recieve<Greet>())
			g.print();

		for (auto& g : server->recieve<Goodbye>())
			g.print();

		using namespace std::chrono_literals;
		std::this_thread::sleep_for(500ms);
	}
}

int main() {

	ezm::ServerFinder finder;
	auto server = finder.connectToAny(2s);

	auto search = finder.search<Greet>(1.5s);

	std::list<ServerInfo> servers;
	while(servers.size() < 2)
	{
		servers = search.snapshot();
		print(servers);
	}

	auto server = servers.top().connect();
	


	finder.searchFor(2s, Port(12345));
	auto servers = ezm::findAllServers(ID<TestGame>,Timeout(2s), Limit(1), Port(12345));
	if (servers.empty()) return;

	try
	{
		auto server = ezm::connect(servers.top());
	}catch(ezm::NoConnection err)
	{
		
	}
	



	Client client;
	std::thread t(&print_thread, &client);

	while (true)
		switch (_getch())
		{
		case '1':
			server.send(Greet());
			break;
		case'2':
			server.send(Goodbye("pozdrowionka"));
			break;
		case'3':
			server.send(Goodbye("nie pozdrawiam"));
			break;
		case'q': return 0;
		}

	//t.join();
	return 0;
}