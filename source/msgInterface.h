#pragma once
#include "ezNetwork.h"
using namespace ezNetwork;
using namespace internal;
#include <mutex>

struct MsgQueue
{
	using ID = DataFlow::ID;
	using Byte = char;
	using Data = std::vector<Byte>;
	using Messages = std::unordered_map<DataFlow::ID, std::vector<Data>>;
	Messages messages;
	std::mutex mutex;

	void add(const DataFlow::Msg& msg){
		Data data(sizeof(msg.size) + sizeof(msg.id) + msg.size);

		auto bufferSize = data.size();
		auto buffer = data.data();

		memcpy_s(buffer, bufferSize, &msg.size, sizeof(msg.size));
		buffer += sizeof(msg.size); bufferSize -= sizeof(msg.size);
		memcpy_s(buffer, bufferSize, &msg.id, sizeof(msg.id));
		buffer += sizeof(msg.id); bufferSize -= sizeof(msg.id);
		memcpy_s(buffer, bufferSize, &msg.data, msg.size);

		std::lock_guard<std::mutex> lock(mutex);
		messages[msg.id].emplace_back(std::move(data));
	}

	std::vector<Data> get(const ID& id){
		std::vector<Data> data;
		{
			std::lock_guard<std::mutex> lock(mutex);
			auto& it = messages.find(id);
			if (it == messages.end()) return data;
			data.swap(it->second);
		}
		return data;
	}

	Messages get(){
		Messages m;
		std::lock_guard<std::mutex> lock(mutex);
		messages.swap(m);
		return m;
	}
};

struct MsgInterface : virtual DataFlow {

	MsgQueue toSend, recieved;

	void send_raw(const Msg& msg) override {
		toSend.add(msg);
	}

	void recieve_raw(const ID& id, VectorI& target) override {
		const auto data = recieved.get(id);

		target.reserve(data.size());
		for (const auto& msg : data)
			target.add(msg.data() + sizeof(ID) + sizeof(Size));
	}
};