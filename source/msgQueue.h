#pragma once
#include <vector>
#include <unordered_map>
#include <mutex>

struct Msg
{
	using ID = unsigned;
	using Size = unsigned;
	using Byte = char;
	using DataPtr = const Byte*;
	const std::vector<Byte> buffer;

	struct Header {
		Size size;
		ID id;
	};

	Msg(const Size& size) 
		: buffer(sizeof(Header) + size){}

	operator char*() const{
		return (char*)buffer.data();
	}

	Msg(const Size& size, const ID& id, const void* data) 
		: buffer(sizeof(Header) + size) 
	{
		auto& h = header();
		h.size = size;
		h.id = id;
		memcpy_s((void*)this->data(), size, data, size);
	}

	Msg(const Msg&& msg) : buffer(std::move(msg.buffer)) {}

	Header& header() const { return (Header&)buffer[0]; }
	DataPtr data() const { return (DataPtr)&buffer[sizeof(Header)]; }
	Size size() const { return buffer.size(); }
	Size dataSize() const { return header().size; }
	ID id() const { return header().id; }
};

class MsgQueue
{
public:
	using MsgMap = std::unordered_map<Msg::ID, std::vector<Msg>>;
	using MsgVector = std::vector<Msg>;

private:
	MsgMap messages;
	std::mutex mutex;

public:
	
	void add(Msg&& msg) {
		std::lock_guard<std::mutex> lock(mutex);
		messages[msg.id()].emplace_back(std::move(msg));
	}

	MsgVector get(const Msg::ID& id) {
			std::lock_guard<std::mutex> lock(mutex);
			auto it = messages.find(id);
			if (it == messages.end()) return MsgVector();
			return std::move(it->second);
	}

	MsgMap get() {
		std::lock_guard<std::mutex> lock(mutex);
		return std::move(messages);
	}
};