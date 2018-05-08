#pragma once
#include <unordered_map>
#include <mutex>
#include <atomic>

#include "message.h"

template<typename Data>
struct ConcurrentList {
	
	void add(Data&& msg) {
		std::lock_guard<std::mutex> lock(mutex);
		messages.emplace_back(std::move(msg));
	}

	std::list<Data> get() {
		std::lock_guard<std::mutex> lock(mutex);
		return std::move(messages);
	}

	private:
		std::mutex mutex;
		std::list<Data> messages;
};

struct ConncurrentMap {
	using Container = std::list<Msg>;
	using Map = std::unordered_map<Msg::Type, ConcurrentList<Msg>>;

	void add(Msg&& msg) {
		const auto& id = msg.id();

		auto found = messages.find(id);
		if (found == messages.end())
		{
			std::lock_guard<std::mutex> lock(mutex);
			return messages[id].add(std::move(msg));
		}
		return found->second.add(std::move(msg));
	}

	template <class Type> Container get() {
		auto id = Msg::type<Type>();
		auto found = messages.find(id);
		if (found == messages.end())
		{
			std::lock_guard<std::mutex> lock(mutex);
			return messages[id].get();
		}
		return found->second.get();
	}

	Container get() {
		Container container;
		std::lock_guard<std::mutex> lock(mutex);
		for (auto& pair : messages) container.splice(container.end(), pair.second.get());
		return container;
	}

	private:
	Map messages;
	std::mutex mutex;
};
