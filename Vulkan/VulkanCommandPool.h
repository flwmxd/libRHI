//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#pragma once

#include "VulkanHelper.h"

namespace maple
{
	class VulkanCommandPool
	{
	  public:
		VulkanCommandPool(int32_t queueIndex, VkCommandPoolCreateFlags flags);
		~VulkanCommandPool();
		auto reset() -> void;

		autoUnpack(commandPool);

		inline auto getHandle() const
		{
			return commandPool;
		}

	  private:
		VkCommandPool commandPool;
	};
};        // namespace maple
