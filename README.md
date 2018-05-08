# ezNet lan c++ library, for creating multiplayer games 

Example Usage:

Messages:
```cpp
struct Greet {
	int i = 5;
	explicit Greet(int i) : i(i){}
	void print() const { printf("Hello from nr: %d\n", i); }
};

struct Text100 {
	char text[100]{};

	Text100(const char* c) { memcpy_s(text, sizeof(text), c, strlen(c) + 1); }

	void print() const { printf("%s\n", text); }
};


```

Server:
```cpp
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
```

Clients: 
```cpp
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
```

