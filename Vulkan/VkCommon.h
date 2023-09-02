//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <vulkan/vulkan.h>
#ifdef USE_VMA_ALLOCATOR
#	include "vk_mem_alloc.h"
#endif        // USE_VMA_ALLOCATOR

namespace maple
{
	class VulkanDevice;
	class VulkanBuffer;
	class VulkanImage;
	class VulkanImageView;
	class VulkanInstance;
	class VulkanQueryPool;
	class VulkanSurface;
	class VulkanSwapChain;
	class VulkanDescriptorPool;
	class VulkanFence;
	class VulkanRenderPass;
	class VulkanFrameBuffer;
	class VulkanShader;
	class VulkanCommandPool;
	class VulkanCommandBuffer;

	auto loadVKRayTracingPipelineKHR(VkInstance instance, PFN_vkGetInstanceProcAddr getInstanceProcAddr, VkDevice device, PFN_vkGetDeviceProcAddr getDeviceProcAddr) -> int32_t;
	auto loadVKAccelerationStructureKHR(VkInstance instance, PFN_vkGetInstanceProcAddr getInstanceProcAddr, VkDevice device, PFN_vkGetDeviceProcAddr getDeviceProcAddr) -> int32_t;
}        // namespace maple

#define MAPLE_ASSERT(condition, ...)                                                         \
	{                                                                                        \
		if (!(condition))                                                                    \
		{                                                                                    \
			LOGE("Assertion Failed : {0} . {1} : {2}", __VA_ARGS__, __FUNCTION__, __LINE__); \
		}                                                                                    \
	}

#	define PROFILE_FUNCTION()