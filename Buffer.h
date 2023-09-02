//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include <cstdint>
#include <memory>
#include <cassert>

namespace maple
{
	struct Buffer
	{
		uint8_t* data;
		uint32_t size;

		Buffer() :
			data(nullptr),
			size(0)
		{
		}

		Buffer(void* data, uint32_t size) :
			data((uint8_t*)data),
			size(size)
		{
		}

		static Buffer copy(const void* data, uint32_t size)
		{
			Buffer buffer;
			buffer.allocate(size);
			memcpy(buffer.data, data, size);
			return buffer;
		}

		inline auto allocate(uint32_t size) -> void
		{
			if (data != nullptr)
			{
				delete[] data;
			}
			data = nullptr;

			if (size == 0)
				return;

			data = new uint8_t[size];
			this->size = size;
		}

		inline auto release()
		{
			delete[] data;
			data = nullptr;
			size = 0;
		}

		inline auto initializeEmpty()
		{
			if (data)
				memset(data, 0, size);
		}

		template <typename T>
		inline auto& read(uint32_t offset = 0)
		{
			return *(T*)((uint8_t*)data + offset);
		}

		inline auto readBytes(uint32_t size, uint32_t offset)
		{
			assert(offset + size <= this->size, "Index out of bounds");
			uint8_t* buffer = new uint8_t[size];
			memcpy(buffer, (uint8_t*)data + offset, size);
			return buffer;
		}

		inline auto write(const void* buffer, uint32_t size, uint32_t offset = 0)
		{
			assert(offset + size <= this->size, "Index out of bounds");
			memcpy((uint8_t*)data + offset, buffer, size);
		}

		inline operator bool() const
		{
			return data;
		}

		inline auto operator[](int index) -> uint8_t&
		{
			return ((uint8_t*)data)[index];
		}

		inline auto operator[](int index) const
		{
			return ((uint8_t*)data)[index];
		}

		template <typename T>
		inline auto as()
		{
			return (T*)data;
		}

		inline auto getSize() const
		{
			return size;
		}
	};
}        // namespace maple
