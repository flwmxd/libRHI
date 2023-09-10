//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Sampler.h"
#include <vulkan/vulkan.h>
namespace maple
{
	class VulkanSampler : public Sampler
	{
	public:
		VulkanSampler(TextureFilter filter, TextureWrap wrapU, TextureWrap wrapV, float maxAnisotropy, uint32_t mipmap);
		~VulkanSampler();
		inline auto getSampler() const { return sampler; }
	private:
		VkSampler sampler = nullptr;
	};
} // namespace maple