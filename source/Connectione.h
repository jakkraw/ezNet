#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

#include <thread>
#include <atomic>
#include <chrono>

#include "msgQueue.h"

struct Connectione {
	MsgQueue &recieved, &toSend;
	SOCKET socket;
	std::atomic<bool> running = true;
	std::thread sender, reciever;

	Connectione(const SOCKET& socket, MsgQueue& recieved, MsgQueue& toSend)
		:
		recieved(recieved),
		toSend(toSend),
		socket(socket),
		sender(&Connectione::sender_thread, this),
		reciever(&Connectione::reciever_thread, this) {
		sender.detach();
		reciever.detach();
	}

	void sender_thread() { while (running) { send(toSend.getBlocking()); } }

	void send(const MsgQueue::MsgMap&& messages) {
		for (const auto& it : messages) for (const auto& msg : it.second) if (!send(msg)) return;
	}

	bool send(const Msg& msg) { return ::send(socket, msg.buffer.data(), msg.size(), 0) > SOCKET_ERROR; }

	void reciever_thread() {
		while (running)
		{
			recieve();
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(0s);
		}
	}

	void recieve() {
		Msg::Size size;
		if (recv(socket, (char*)&size, sizeof(size), MSG_PEEK) == sizeof(Msg::Size))
		{
			Msg msg(size);
			if (recv(socket, (char*)msg.buffer.data(), msg.size(), MSG_WAITALL) == (int)msg.size())
			{
				msg.setSender(socket);
				recieved.add(std::move(msg));
			}
		}
	}

	~Connectione() {
		running = false;
		shutdown(socket, SD_BOTH);
		closesocket(socket);
	}
};

#include <list>


struct WinSockLifetime {
	WinSockLifetime() { if (WSAStartup(MAKEWORD(2, 2), &WSADATA())) throw; }
	~WinSockLifetime() { WSACleanup(); }
};

extern const WinSockLifetime lifetime = WinSockLifetime();
