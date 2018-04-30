#pragma once
#include <vector>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <chrono>

struct Msg
{
	using Type = size_t;
	using Sender = size_t;
	using Size = size_t;
	using Byte = char;
	using Data = const Byte*;
	std::vector<Byte> buffer;

	using Clock = std::chrono::system_clock;
	using Timestamp = std::chrono::time_point<Clock,std::chrono::nanoseconds>;

	struct Header {
		Size size;
		Type type;
		Timestamp timestamp;
		Sender sender;
	};

	Msg(const Size& size) 
		: buffer(sizeof(Header) + size){}

	Msg(const Size& size, const Type& id, const void* data) 
		: buffer(sizeof(Header) + size) 
	{
		auto& h = header();
		h.size = size;
		h.type = id;
		h.timestamp = Clock::now();
		memcpy_s((void*)payload(), size, data, size);
	}

	Msg(Msg&& msg) noexcept : buffer(std::move(msg.buffer)) {}

	void setSender(Sender sender) { header().sender = sender; }
	Header& header() const { return (Header&)buffer[0]; }
	Data payload() const { return Data(&buffer[sizeof(Header)]); }
	Size size() const { return buffer.size(); }
	Size dataSize() const { return header().size; }
	Type id() const { return header().type; }
};

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

	MsgMap getBlocking() {
		std::unique_lock<std::mutex> lock(m);
		cv.wait(lock, [this] {return hasData.load(); });
		hasData = false;
		return std::move(messages);
	}
};