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
	using Clock = std::chrono::system_clock;
	using Timestamp = std::chrono::time_point<Clock, std::chrono::nanoseconds>;

	struct Header {
		Size size;
		Type type;
		Timestamp timestamp;
		Sender sender;
	};

	std::vector<Byte> buffer;

	template<typename Data>
	static constexpr Type type() { return typeid(Data).hash_code(); }
	template<typename Data>
	static constexpr Size size() { return sizeof(Data); }

	template<typename Data>
	static Msg toMsg(const Data& data) {
		return { size<Data>(),type<Data>(), &data };
	}

	explicit Msg(const Size& size) : buffer(sizeof(Header) + size) {}

	Msg(const Size& size, const Type& type, const void* data) : Msg(size)
	{
		setSize(size);
		setType(type);
		setTimestamp(Clock::now());
		setPayload(data, size);
	}

	Msg(Msg&& msg) = default;
	Msg(const Msg& msg) = default;

	void setSender(Sender sender) { header().sender = sender; }
	void setSize(Size size) { header().size = size; }
	void setType(Type type) { header().type = type; }
	void setTimestamp(Timestamp timestamp) { header().timestamp = timestamp; }
	void setPayload(const void* data, const Size size) {
		memcpy_s(&buffer[sizeof(Header)], size, data, buffer.size() - sizeof(Header));
	}
	
	Header& header() const { return (Header&)buffer[0]; }
	Data payload() const { return Data(&buffer[sizeof(Header)]); }
	Size size() const { return static_cast<Size>(buffer.size()); }
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
