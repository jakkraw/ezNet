#pragma once
#include <vector>
#include <chrono>

struct Msg
{
	using Type = size_t;
	using Sender = size_t;
	using Size = unsigned;
	using Byte = char;
	using Data = Byte*;
	std::vector<Byte> buffer;

	template<typename Msg>
	static constexpr Type type() { return typeid(Msg).hash_code(); }
	template<typename Msg>
	static constexpr Size size() { return sizeof(Msg); }

	using Clock = std::chrono::system_clock;
	using Timestamp = std::chrono::time_point<Clock, std::chrono::nanoseconds>;

	struct Header {
		Size size;
		Type type;
		Timestamp timestamp;
		Sender sender;
	};

	template<typename Data>
	static Msg toMsg(const Data& data) {
		return{ size<Data>(),type<Data>(), &data };
	}

	explicit Msg(const Size& size)
		: buffer(sizeof(Header) + size) {}

	Msg(const Size& size, const Type& type, const void* data)
		: buffer(sizeof(Header) + size)
	{
		auto& h = header();
		h.size = size;
		h.type = type;
		h.timestamp = Clock::now();
		memcpy_s(payload(), size, data, size);
	}

	Msg(Msg&& msg) noexcept : buffer(std::move(msg.buffer)) {}

	Msg(const Msg& msg) noexcept : buffer(msg.buffer) {}

	void setSender(Sender sender) { header().sender = sender; }
	Header& header() const { return (Header&)buffer[0]; }
	Data payload() const { return Data(&buffer[sizeof(Header)]); }
	Size size() const {
		return static_cast<Size>(buffer.size());
	}
	Size payloadSize() const { return header().size; }
	Type id() const { return header().type; }

	operator const char*() const { return buffer.data(); }
	operator char*() { return buffer.data(); }

	template<typename Data>
	const Data& payloadAs() const { return reinterpret_cast<const Data&>(*payload()); }

	Sender sender() const {
		return header().sender;
	}
};
