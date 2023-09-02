//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#pragma once

#include "DescriptorSet.h"
#include "Console.h"
#include "Definitions.h"
#include <string>

namespace maple
{
	struct  BufferElement
	{
		std::string name;
		Format      format;
		uint32_t    offset     = 0;
		bool        normalized = false;
	};

	class  BufferLayout
	{
	  private:
		uint32_t                   size = 0;
		std::vector<BufferElement> layouts;

	  public:
		BufferLayout();
		auto reset() -> void;

		template <typename T>
		auto push(const std::string &name, uint32_t level, bool normalized = false) -> void
		{
			MAPLE_ASSERT(false, "Unknown type!");
		}

		inline auto &getLayout() const
		{
			return layouts;
		}

		inline auto &getLayout()
		{
			return layouts;
		}

		inline auto getStride() const
		{
			return size;
		}

		auto computeStride() -> void;

	  private:
		auto push(const std::string &name, Format format, uint32_t size, uint32_t location, bool normalized) -> void;
	};

	template <>
	auto  BufferLayout::push<float>(const std::string &name, uint32_t level, bool normalized) -> void;
	template <>
	auto  BufferLayout::push<uint32_t>(const std::string &name, uint32_t level, bool normalized) -> void;
	template <>
	auto  BufferLayout::push<uint8_t>(const std::string &name, uint32_t level, bool normalized) -> void;
	template <>
	auto  BufferLayout::push<vec2>(const std::string &name, uint32_t level, bool normalized) -> void;
	template <>
	auto  BufferLayout::push<vec3>(const std::string &name, uint32_t level, bool normalized) -> void;
	template <>
	auto  BufferLayout::push<vec4>(const std::string &name, uint32_t level, bool normalized) -> void;

	template <>
	auto  BufferLayout::push<int32_t>(const std::string &name, uint32_t level, bool normalized) -> void;
	template <>
	auto  BufferLayout::push<ivec2>(const std::string &name, uint32_t level, bool normalized) -> void;
	template <>
	auto  BufferLayout::push<ivec3>(const std::string &name, uint32_t level, bool normalized) -> void;
	template <>
	auto  BufferLayout::push<ivec4>(const std::string &name, uint32_t level, bool normalized) -> void;

}        // namespace maple
