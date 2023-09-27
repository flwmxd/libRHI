//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#include "VulkanUniformBuffer.h"
#include "Console.h"
#include <memory.h>
namespace maple
{
	VulkanUniformBuffer::VulkanUniformBuffer(uint32_t size, const void *data)
	{
		VulkanBuffer::init(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, size, data);
	}

	VulkanUniformBuffer::VulkanUniformBuffer()
	{
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
	}

	auto VulkanUniformBuffer::init(uint32_t size, const void *data) -> void
	{
		PROFILE_FUNCTION();
		VulkanBuffer::init(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, size, data);
	}

	auto VulkanUniformBuffer::setDynamicData(uint32_t size, const void *data, uint32_t offset) -> void
	{
		PROFILE_FUNCTION();
		VulkanBuffer::map();
		memcpy((uint8_t*)(mapped)+offset, data, size);
		VulkanBuffer::flush(size);
		VulkanBuffer::unmap();
	}

	auto VulkanUniformBuffer::setData(uint32_t size, const void *data) -> void
	{
		PROFILE_FUNCTION();
		VulkanBuffer::map();
		memcpy(mapped, data, size);
		VulkanBuffer::unmap();
	}
};        // namespace maple