/////////////////////////////////////////	/////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#include "VulkanBatchTask.h"
#include "Raytracing/VulkanAccelerationStructure.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDebug.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"
#include <cmath>

namespace maple
{
#ifdef MAPLE_VULKAN

	static inline size_t align(size_t x, size_t alignment)
	{
		return (x + (alignment - 1)) & ~(alignment - 1);
	}

	auto VulkanBatchTask::execute(const CommandBuffer* cmd) -> void
	{
		auto vkCmd = static_cast<const VulkanCommandBuffer*>(cmd);

		auto alignment = VulkanDevice::get()->getPhysicalDevice()->getAccelerationStructureProperties().minAccelerationStructureScratchOffsetAlignment;

		if (requests.size() > 0)
		{
			//std::shared_ptr<VulkanBuffer> scratchBuffer;
			debug_utils::cmdBeginLabel("Build Bottom AccelerationStructure");
			VkMemoryBarrier memoryBarrier;
			memoryBarrier.sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			memoryBarrier.pNext         = nullptr;
			memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
			memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

			VkDeviceSize scratchBufferSize = 0;

			for (auto &request : requests)
				scratchBufferSize += align(request.accelerationStructure->getBuildScratchSize(),request.accelerationStructure->getBuildSizes().buildScratchSize);

			if (scratchBuffer == nullptr) 
			{
				scratchBuffer = std::make_shared<VulkanBuffer>(
					VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
					scratchBufferSize, nullptr, VMA_MEMORY_USAGE_GPU_ONLY, 0);
			}
			else if(scratchBuffer->getSize() < scratchBufferSize)
			{
				scratchBuffer->resize(scratchBufferSize,nullptr);
			}

			

			for (auto &request : requests)
			{
				const VkAccelerationStructureBuildRangeInfoKHR *buildRanges = &request.buildRanges[0];

				VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};

				buildInfo.sType                     = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
				buildInfo.type                      = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
				buildInfo.flags                     = request.accelerationStructure->getFlags();
				buildInfo.srcAccelerationStructure  = VK_NULL_HANDLE;
				buildInfo.dstAccelerationStructure  = request.accelerationStructure->getAccelerationStructure();
				buildInfo.geometryCount             = (uint32_t) request.geometries.size();
				buildInfo.pGeometries               = request.geometries.data();
				buildInfo.scratchData.deviceAddress = scratchBuffer->getDeviceAddress() 
					+ align(request.accelerationStructure->getBuildScratchSize(), request.accelerationStructure->getBuildSizes().buildScratchSize);;
					//request.accelerationStructure->getDeviceAddress();
					//scratchBuffer->getDeviceAddress();

				vkCmdBuildAccelerationStructuresKHR(vkCmd->getCommandBuffer(), 1, &buildInfo, &buildRanges);
				vkCmdPipelineBarrier(vkCmd->getCommandBuffer(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &memoryBarrier, 0, 0, 0, 0);
			}
			debug_utils::cmdEndLabel();
			requests.clear();
		}
	}

	auto VulkanBatchTask::buildBlas(VulkanAccelerationStructure *                               accelerationStructure,
	                                const std::vector<VkAccelerationStructureGeometryKHR> &     geometries,
	                                const std::vector<VkAccelerationStructureBuildRangeInfoKHR> buildRanges) -> void
	{
		if (geometries.size() > 0 || buildRanges.size() > 0)
			requests.push_back({accelerationStructure, geometries, buildRanges});
		else
		{
			throw std::runtime_error("(Vulkan) Building a BLAS fail.");
		}
	}
#endif        // MAPLE_VULKAN
}        // namespace maple