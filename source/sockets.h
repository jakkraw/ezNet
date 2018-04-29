#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

#include <thread>
#include <atomic>

#include "msgQueue.h"

struct Connection {
	MsgQueue& recieved, &toSend;
	SOCKET& socket;
	std::atomic<bool> running = true;
	std::thread sender, reciever;

	Connection(SOCKET& socket, MsgQueue& recieved, MsgQueue& toSend) :
		recieved(recieved),
		toSend(toSend),
		socket(socket),
		sender(&Connection::sender_thread, this),
		reciever(&Connection::reciever_thread, this)
	{
		sender.detach();
		reciever.detach();
	}

	void sender_thread()
	{
		while(running){
			send(toSend.get());
		}
	}

	void send(const MsgQueue::MsgMap&& messages)
	{
		for (const auto& it : messages)
			for (const auto& data : it.second)
				if(!send(data)) return;
	}

	bool send(const Msg& data){
		return ::send(socket, data.data(), data.size(), 0) > SOCKET_ERROR;
	}

	void reciever_thread()
	{
		while(running)
			recieve();
	}

	void recieve()
	{
		Msg::Size size;
		auto bytes = ::recv(socket, (char*)&size, sizeof(size), MSG_PEEK | MSG_WAITALL);
		if (bytes < sizeof(Msg::Size)) return;

		Msg msg(size);
		bytes = ::recv(socket, msg, msg.size(), MSG_WAITALL);
		if (bytes < (int)msg.size()) return;

		recieved.add(std::move(msg));
	}

	~Connection(){
		running = false;
		shutdown(socket, SD_BOTH);
		closesocket(socket);
	}

};




#include <list>
std::list<std::string> myIPList()
{
	std::list<std::string> ipList;
	char hostName[80];
	if (gethostname(hostName, sizeof(hostName)) == SOCKET_ERROR) return ipList;

	const auto host = gethostbyname(hostName);
	if (!host) return ipList;

	for (int i = 0; host->h_addr_list[i] != nullptr; ++i) {
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



