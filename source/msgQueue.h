#pragma once
#include <vector>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include "msg.h"
#include <atomic>

class MsgQueue
{
public:
	using MsgMap = std::unordered_map<Msg::Type, std::vector<Msg>>;
	using MsgVector = std::vector<Msg>;

private:
	MsgMap messages;
	std::condition_variable cv;
	std::mutex m;
	std::atomic<bool> hasData = false;

public:
	
	void add(Msg&& msg) {
		{
			std::lock_guard<std::mutex> lock(m);
			messages[msg.id()].emplace_back(std::move(msg));
			hasData = true;
		}
		cv.notify_all();
	}

	MsgVector get(const Msg::Type& id) {
			std::lock_guard<std::mutex> lock(m);
			auto it = messages.find(id);
			if (it == messages.end()) return MsgVector();
			return std::move(it->second);
	}

	MsgMap get() {
		std::lock_guard<std::mutex> lock(m);
		hasData = false;
		return std::move(messages);
	}

	MsgMap getAny() {
		std::unique_lock<std::mutex> lock(m);
		cv.wait(lock, [this] {return hasData.load(); });
		hasData = false;
		return std::move(messages);
	}
};
