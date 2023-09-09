//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////

#include "VulkanSampler.h"
#include "VulkanHelper.h"
#include "VulkanDevice.h"

namespace maple
{
	VulkanSampler::VulkanSampler(TextureFilter filter, TextureWrap wrapU, TextureWrap wrapV, float maxAnisotropy)
	    : Sampler(filter, wrapU, wrapV, maxAnisotropy)
	{
		
		VkSamplerCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		createInfo.magFilter = VkConverter::textureFilterToVK(filter);
		createInfo.minFilter = VkConverter::textureFilterToVK(filter);
		createInfo.mipmapMode = VkConverter::textureMipmapFilterToVK(filter);
		createInfo.addressModeU = VkConverter::textureWrapToVK(wrapU);
		createInfo.addressModeV = VkConverter::textureWrapToVK(wrapV);
		createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		createInfo.minLod = 0;
		createInfo.maxLod = 16;

		VK_CHECK_RESULT(vkCreateSampler(*VulkanDevice::get(), &createInfo, 0, &sampler));
	}
}