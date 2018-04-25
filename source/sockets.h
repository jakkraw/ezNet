#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

#include <thread>
#include <atomic>

struct MsgWrapper
{
	using Byte = char;
	using ID = internal::DataFlow::ID;
	using Size = internal::DataFlow::Size;
	Size size;
	ID id;
};

struct Connection {
	MsgQueue revieved, toSend;
	SOCKET socket;
	std::atomic<bool> running = true;
	std::thread sender, reciever;
	
	Connection(SOCKET socket) : 
		socket(socket),
		sender(&Connection::sender_thread, this),
		reciever(&Connection::reciever_thread, this)
	{
		sender.detach();
		reciever.detach();
	}

	void sender_thread()
	{
		while(running)
		{
			const auto messages = toSend.get();

			for (const auto& it : messages)
				for (const auto& data : it.second)
					send(data);
				
		}
	}

	void send(const MsgQueue::Data& data)
	{
		::send(socket, data.data(), data.size(), 0);
	}

	struct Reciever
	{
		using Byte = MsgQueue::Byte;
		using Data = MsgQueue::Data;
		using Size = DataFlow::Size;
		using Msg = DataFlow::Msg;
		using ID = DataFlow::ID;
		enum class State
		{
			Clean, PartiallyRecieved, AnotherInQueue
		};
		State state = State::Clean;
		Byte buffer[1024];
		SOCKET& socket;
		Reciever(SOCKET socket) : socket(socket){}

		void clean(MsgQueue& recieved)
		{
			const auto size = ::recv(socket, buffer, sizeof(buffer), 0);
			if (size <= 0) return;
			if (size < sizeof(Size)) return;

			auto& msgSize = *(Size*)buffer;
			auto totalMsgSize = msgSize + sizeof(ID) + sizeof(Size);

			if (size < totalMsgSize) {
				state = State::PartiallyRecieved;
				return;
			}
			if(size > totalMsgSize)
			{
				state = State::AnotherInQueue;
				return;
			}


			recieved.add(Msg(msgSize,))


		}

		void recieve(MsgQueue& recieved)
		{
			

			switch(state)
			{
			case State::Clean:
				clean(recieved);
				break;
			case State::AnotherInQueue:
				break;
			case State::PartiallyRecieved:
				break;
			}

		
		}
	};


	void reciever_thread()
	{
		Reciever reciever(socket);

		while(running)
		{
			
			reciever.recieve();

		}
	}


	~Connection()
	{
		running = false;
		shutdown(socket, SD_BOTH);
		closesocket(socket);
	}



};

SOCKET createClientSocket(Address server)
{
	SOCKET socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(server.ip);
	addr.sin_port = htons(server.port);
	::connect(socket, (sockaddr*)&addr, sizeof(addr));
	return socket;
}


#include <list>
std::list<std::string> myIPList()
{
	std::list<std::string> ipList;
	char hostName[80];
	if (gethostname(hostName, sizeof(hostName)) == SOCKET_ERROR) return ipList;

	const auto host = gethostbyname(hostName);
	if (!host) return ipList;

	for (int i = 0; host->h_addr_list[i] != 0; ++i) {
		in_addr address;
		memcpy(&address, host->h_addr_list[i], sizeof(in_addr));
		ipList.emplace_back(inet_ntoa(address));
	}

	return ipList;
}

struct WinSockLifetime {
	WinSockLifetime() {
		if (WSAStartup(MAKEWORD(2, 2), &WSADATA())) throw;
	}
	~WinSockLifetime() { WSACleanup(); }
};
extern const WinSockLifetime lifetime = WinSockLifetime();



