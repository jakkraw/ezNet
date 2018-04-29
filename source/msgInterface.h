#pragma once
#include "interface/internal.h"
#include "msgQueue.h"

struct ezMsgFlow : virtual ezm::internal::MsgFlow {

	MsgQueue toSend, recieved;

	void send(const Msg& msg) override {
		toSend.add(msg);
	}

	void recieve_raw(const ID& id, VectorI& target) override {
		const auto data = recieved.get(id);

		target.reserve(data.size());
		for (const auto& msg : data)
			target.add(msg.data() + sizeof(Header));
	}
};