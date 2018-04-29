#include "../messages.h"
#include "../../../source/interface/ezNetwork.h"
#include <thread>
#include <chrono>
#include <conio.h>
#pragma comment(lib,"ezNetwork.lib")
using namespace ezm;

#include <thread>



void print_thread(Server* server)
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
	Server server;

	std::thread t(&print_thread, &server);
	
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
	
	
	return 0;
}