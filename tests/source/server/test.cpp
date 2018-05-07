#include <thread>
#include <chrono>
#include <conio.h>
//#pragma comment(lib,"ezNetwork.lib")

#include "../messages.h"
#include "../../../source/server.h"

void printer(Server& server, std::atomic_bool& active)
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

int main() {
	Server server;
	std::atomic<bool> active = true;
	std::thread t(&printer, std::ref(server), std::ref(active));
	
	while (true)
		switch (_getch())
		{
		case '1':
			server.send(Greet());
			break;
		case'2':
			server.send(Goodbye("Test"));
			break;
		case'3':
			server.send(Goodbye("Server Message"));
			break;
		case'q': 
			active = false;
			t.join();
			return 0;
		}
}