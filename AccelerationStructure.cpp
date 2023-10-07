//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#include "AccelerationStructure.h"
#include "StorageBuffer.h"

#ifdef MAPLE_VULKAN
#	include "Vulkan/VkCommon.h"
#	include "Vulkan/VulkanContext.h"
#	include "Vulkan/Raytracing/RayTracingProperties.h"
#	include "Vulkan/Raytracing/VulkanAccelerationStructure.h"
#	include "Vulkan/VulkanBuffer.h"
#	include "Vulkan/VulkanVertexBuffer.h"
#	include "Vulkan/VulkanIndexBuffer.h"
#else
#endif        // MAPLE_VULKAN

namespace maple
{
	auto AccelerationStructure::createTopLevel(const uint32_t maxInstanceCount) -> Ptr
	{
#ifdef MAPLE_VULKAN
		return std::make_shared<VulkanAccelerationStructure>(maxInstanceCount);
#else
		return std::make_shared<NullAccelerationStructure>();
#endif        // MAPLE_VULKAN
	}

	auto AccelerationStructure::createBottomLevel(const VertexBuffer::Ptr &vertexBuffer, const IndexBuffer::Ptr &indexBuffer, uint32_t vertexCount, uint32_t vertexStride, BatchTask::Ptr batchTask) -> Ptr
	{
#ifdef MAPLE_VULKAN
		return std::make_shared<VulkanAccelerationStructure>(vertexBuffer->getAddress(), indexBuffer->getAddress(), vertexCount, indexBuffer->getCount(), vertexStride, batchTask);
#else
		return std::make_shared<NullAccelerationStructure>();
#endif
	}

	auto AccelerationStructure::createBottomLevel(const VertexBuffer* vertexBuffer, const IndexBuffer* indexBuffer, uint32_t vertexCount, uint32_t vertexStride, BatchTask::Ptr batchTask) ->Ptr
	{
#ifdef MAPLE_VULKAN
		return std::make_shared<VulkanAccelerationStructure>(vertexBuffer->getAddress(), indexBuffer->getAddress(), vertexCount, indexBuffer->getCount(), vertexStride, batchTask);
#else
		return std::make_shared<NullAccelerationStructure>();
#endif
	}

}        // namespace maple