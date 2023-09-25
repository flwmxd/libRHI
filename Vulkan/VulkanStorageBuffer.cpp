//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////

#include "VulkanStorageBuffer.h"
#include "VulkanBuffer.h"
#include "Console.h"
namespace maple
{
	VulkanStorageBuffer::VulkanStorageBuffer(uint32_t size, const void *data, const BufferOptions &options) :
	    options(options)
	{
		accessFlagBits = VK_ACCESS_SHADER_READ_BIT;

		VkBufferUsageFlags flags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

		if (options.indirect)
		{
			flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
			accessFlagBits |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
		}

		lastAccessFlagBits = accessFlagBits;
		if (data == nullptr)
		{
			std::vector<uint8_t> newData(size,0);
			vulkanBuffer = std::make_shared<VulkanBuffer>(flags, size, newData.data(), options.vmaUsage, options.vmaCreateFlags);
		}
		else
		{
			vulkanBuffer = std::make_shared<VulkanBuffer>(flags, size, data, options.vmaUsage, options.vmaCreateFlags);
		}
	}

	VulkanStorageBuffer::VulkanStorageBuffer(const BufferOptions &options) :
	    options(options)
	{
		vulkanBuffer = std::make_shared<VulkanBuffer>();
	}

	VulkanStorageBuffer::VulkanStorageBuffer(uint32_t size, uint32_t flags, const BufferOptions &options)
	{
		vulkanBuffer = std::make_shared<VulkanBuffer>(flags, size, nullptr, options.vmaUsage, options.vmaCreateFlags);
	}

	auto VulkanStorageBuffer::setData(uint32_t size, const void *data) -> void
	{
		PROFILE_FUNCTION();

		if (vulkanBuffer->getSize() == 0)
		{
			VkBufferUsageFlags flags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			accessFlagBits           = VK_ACCESS_SHADER_READ_BIT;

			if (options.indirect)
			{
				flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
				accessFlagBits |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
			}

			lastAccessFlagBits = accessFlagBits;
			vulkanBuffer->init(flags, size, data, options.vmaUsage, options.vmaCreateFlags);
		}
		else
		{
			vulkanBuffer->setVkData(size, data);
		}
	}

	auto VulkanStorageBuffer::getHandle() const -> VkBuffer &
	{
		return vulkanBuffer->getVkBuffer();
	}

	auto VulkanStorageBuffer::mapMemory(const std::function<void(void *)> &call) -> void
	{
		PROFILE_FUNCTION();
		vulkanBuffer->map();
		call(vulkanBuffer->getMapped());
		vulkanBuffer->unmap();
	}

	auto VulkanStorageBuffer::unmap() -> void
	{
		PROFILE_FUNCTION();
		vulkanBuffer->unmap();
	}

	auto VulkanStorageBuffer::map() -> void *
	{
		PROFILE_FUNCTION();
		vulkanBuffer->map();
		return vulkanBuffer->getMapped();
	}

	auto VulkanStorageBuffer::getDeviceAddress() const -> uint64_t
	{
		PROFILE_FUNCTION();
		return vulkanBuffer->getDeviceAddress();
	}

	auto VulkanStorageBuffer::getSize() const -> uint32_t
	{
		PROFILE_FUNCTION();
		return vulkanBuffer->getSize();
	}

	auto VulkanStorageBuffer::resize(uint32_t size) -> void
	{
		PROFILE_FUNCTION();
		dirty = true;
		vulkanBuffer->release();

		VkBufferUsageFlags flags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		accessFlagBits           = VK_ACCESS_SHADER_READ_BIT;

		if (options.indirect)
		{
			flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
			accessFlagBits |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
		}

		lastAccessFlagBits = accessFlagBits;
		vulkanBuffer->init(flags, size, nullptr, options.vmaUsage, options.vmaCreateFlags);
	}

	auto VulkanStorageBuffer::setAccessFlagBits(uint32_t flags) -> void
	{
		PROFILE_FUNCTION();
		lastAccessFlagBits = accessFlagBits;
		accessFlagBits     = flags;
	}

	auto VulkanStorageBuffer::isDirty() const -> bool 
	{
		return dirty;
	}

	auto VulkanStorageBuffer::setDirty(bool dirty) -> void 
	{
		this->dirty = dirty;
	}
}        // namespace maple