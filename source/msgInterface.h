#pragma once
#include "ezNetwork.h"
using namespace ezNetwork;
using namespace internal;

struct MsgInterface : virtual DataFlow {

	using Byte = char;
	using Data = std::vector<Byte>;
	using Messages = std::unordered_map<ID, std::vector<Data>>;
	Messages toSend, recieved;

	void send_raw(ID id, Size size, const void* data) override {
		auto& messages = toSend;
		messages[id].emplace_back((Byte*)data, (Byte*)data + size);
	}

	void recieve_raw(ID id, VectorRawRecive& target) override {
		auto& messages = toSend;

		auto& it = messages.find(id);
		if (it == messages.end()) return;

		auto& source = it->second;
		target.reserve(source.size());
		for (const auto& msg : source)
			target.add((void*)msg.data());

		source.clear();
	}
};