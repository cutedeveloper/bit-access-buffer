#pragma once

#include <stdint.h>
#include <stddef.h>
#include <type_traits>

#include <boost/asio/buffer.hpp>

namespace detail
{

	template<typename T, bool = std::is_enum<T>::value>
	struct underlying_type { using type = T; };

	template<typename T>
	struct underlying_type<T, true> : ::std::underlying_type<T> {};
}

class Buffer
{
public:
	Buffer(const uint8_t* data, size_t length)
	: buffer(boost::asio::buffer(data, length))
	{
	}

	bool advanceable(size_t size) const
	{
		return size <= get_size();
	}
	bool advance(size_t size)
	{
		if (!advanceable(size))
			return false;		// Requested to point to outside of the current buffer.

		buffer = buffer + size;
		return true;
	}

	template<typename Type>
	const Type* get_data_as() const
	{
		if (get_size() == 0)
			return nullptr;

		return boost::asio::buffer_cast<const Type*>(buffer);
	}

	const uint8_t* get_data() const
	{
		return get_data_as<uint8_t>();
	}

	template<typename Type>
	bool get_bits(uint8_t offset, uint8_t length, Type* destination)
	{
                typedef typename detail::underlying_type<Type>::type UnderlyingType;

		if (!advanceable(sizeof(Type)))
			return false;

		if (offset + length > sizeof(Type) * 8)
			return false;

		UnderlyingType bitmask = 0;

		for (int i = 0; i < length; ++i)
			bitmask |= (1 << i);

		*destination = static_cast<Type>(*get_data_as<UnderlyingType>() >> offset & bitmask);
		return true;
	}

	template<typename Type>
	bool extract_bytes(Type* destination)
	{
		if (!advanceable(sizeof(Type)))
			return false;

		*destination = *get_data_as<Type>();
		advance(sizeof(Type));
		return true;
	}

	bool extract_bytes(size_t length, uint8_t* destination)
	{
		if (!advanceable(length))
			return false;

		memcpy(destination, get_data(), length);
		advance(length);
		return true;
	}

	size_t get_size() const
	{
		return boost::asio::buffer_size(buffer);
	}
private:
	boost::asio::const_buffer buffer;
};

