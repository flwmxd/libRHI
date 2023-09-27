//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////

#include "VulkanDebug.h"
#include "VulkanCommandBuffer.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"

#include "../GraphicsContext.h"
#include "../SwapChain.h"
#include <memory>
#include <random>

namespace maple
{
	namespace debug_utils
	{
		PFN_vkCmdDebugMarkerBeginEXT beginFunc = nullptr;
		PFN_vkCmdDebugMarkerEndEXT endFunc = nullptr;

		static float colors[1000][4] = {};
		static int32_t lastIndex = -1;
		static int32_t colorIndex = 0;

		auto cmdBeginLabel(const std::string &caption) -> void
		{
			PROFILE_FUNCTION();
			auto index = maple::GraphicsContext::get()->getSwapChain()->getCurrentBufferIndex();

			if(lastIndex != index) 
			{ 
				colorIndex = -1;
				lastIndex = index;
			}

			colorIndex++;

			if(beginFunc == nullptr) 
			{
				beginFunc = (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(*VulkanDevice::get(), "vkCmdDebugMarkerBeginEXT");
				endFunc = (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(*VulkanDevice::get(), "vkCmdDebugMarkerEndEXT");
				std::random_device rd;
				std::mt19937 mt(rd());
				std::uniform_real_distribution<float> dist(0.0, 1.0);
				for(auto i = 0; i < 1000; i++) 
				{ 
					for(auto j = 0; j < 3; j++) { colors[i][j] = dist(mt); }
					colors[i][3] = 1;
				}
			}

			if(beginFunc == nullptr) return;

			auto cmdBuffer = maple::GraphicsContext::get()->getSwapChain()->getCurrentCommandBuffer();
			auto vkCmd = static_cast<VulkanCommandBuffer *>(cmdBuffer);

			VkDebugMarkerMarkerInfoEXT labelInfo{};
			labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
			labelInfo.pMarkerName = caption.c_str();
			memcpy(labelInfo.color, colors[colorIndex], sizeof(float) * 4);
			beginFunc(vkCmd->getCommandBuffer(), &labelInfo);
		}

		auto cmdEndLabel() -> void
		{
			PROFILE_FUNCTION();
			if(beginFunc == nullptr) return;

			auto cmdBuffer = maple::GraphicsContext::get()->getSwapChain()->getCurrentCommandBuffer();
			auto vkCmd = static_cast<VulkanCommandBuffer *>(cmdBuffer);
			endFunc(vkCmd->getCommandBuffer());
		}
	} // namespace debug_utils
} // namespace maple