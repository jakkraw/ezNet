
#include <thread>
#include <chrono>
#include <conio.h>
#pragma comment(lib,"ezNetwork.lib")

#include "../messages.h"
#include "../../../source/Server.h"
std::atomic<bool> active = true;

void print_thread(Server* server)
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
		case'q': 
			active = false;
			t.join();
			return 0;
		}

}