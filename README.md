# ezNet lan c++ library, for creating multiplayer games 

Example Usage:

Messages:
```cpp
struct Greet {
	int i = 5;
	void print() const { printf("Greets with number: %d\n", i); }
};

struct Goodbye {
	char text[20]{0};
	Goodbye(const char* c) { memcpy_s(text, 20, c, strlen(c)+1); }
	void print() const { printf("message with: %s\n", &text); }
};

```

Server:
```cpp
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

```

Clients: 
```cpp
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
```

