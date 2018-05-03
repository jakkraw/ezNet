#pragma once
#include <vector>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include "msg.h"
#include <atomic>

struct List {
	using Container = std::list<Msg>;
	std::mutex m;
	Container messages;

	void add(Msg&& msg) {
		std::lock_guard<std::mutex> lock(m);
		messages.emplace_back(std::move(msg));
	}

	Container get() {
		std::lock_guard<std::mutex> lock(m);
		return std::move(messages);
	}

};

class Queue
{
public:
	using Container = std::list<Msg>;
	using MsgMap = std::unordered_map<Msg::Type, List>;

	void add(Msg&& msg) {
		const auto& id = msg.id();

		auto found = messages.find(id);
		if (found == messages.end()) {
			std::lock_guard<std::mutex> lock(m);
			return messages[id].add(std::move(msg));
		}
		return found->second.add(std::move(msg));
	}

	template<class Type>
	Container get() {
		auto id = Msg::type<Type>();
		auto found = messages.find(id);
		if (found == messages.end()) {
			std::lock_guard<std::mutex> lock(m);
			return messages[id].get();
		}
		return found->second.get();
	}

	Container get() {
		Container container;
		std::lock_guard<std::mutex> lock(m);
		for (auto& pair : messages)
				container.splice(container.end(), pair.second.get());
		return container;
	}

private:
	MsgMap messages;
	std::mutex m;
};