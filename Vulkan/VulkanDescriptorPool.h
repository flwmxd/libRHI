//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../DescriptorPool.h"
#include "VulkanHelper.h"

namespace maple
{
	class VulkanDescriptorPool final : public DescriptorPool
	{
	  public:
		VulkanDescriptorPool(const DescriptorPoolCreateInfo &info);
		~VulkanDescriptorPool();
		autoUnpack(descriptorPool);
		inline auto getHandle() const
		{
			return descriptorPool;
		}
	 private:
		VkDescriptorPool descriptorPool = nullptr;
	};
};        // namespace maple
