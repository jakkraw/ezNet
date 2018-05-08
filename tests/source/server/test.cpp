#include <conio.h>
#include <thread>
#include <chrono>
#include <iostream>
using namespace std::chrono_literals;

#include "../messages.h"
#include "server.h"

void printer(Server& server, std::atomic_bool& active) {
	while (active)
	{
		for (auto&& msg : server.recieve<Greet>()) msg.print();

		for (auto&& msg : server.recieve<Text100>()) msg.print();

		std::this_thread::sleep_for(0s);
	}
}

int main() {
	Server server;
	std::atomic_bool active = true;
	std::thread printer_thread(&printer, std::ref(server), std::ref(active));

	auto nr = 0;
	std::string text;

	while (active)
		switch (_getch())
		{
			case '1': server.send(Greet{ ++nr });
				break;
			case'2': server.send(Text100("Test"));
				break;
			case'3': server.send(Text100("Server Message"));
				break;
			case'\r':
				std::cout << "Enter text message: ";
				std::getline(std::cin,text);	
				server.send(Text100(text.c_str()));
				break;
			case'q': active = false;
				break;
		}

	printer_thread.join();
}
