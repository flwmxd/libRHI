//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#pragma once

#include "../../AccelerationStructure.h"
#include "../VulkanBuffer.h"

namespace maple
{
	class RayTracingProperties;
	class BatchTask;

	class VulkanAccelerationStructure : public AccelerationStructure
	{
	  public:
		struct Desc
		{
			VkAccelerationStructureCreateInfoKHR            createInfo{};
			VkAccelerationStructureBuildGeometryInfoKHR     buildGeometryInfo{};
			std::vector<VkAccelerationStructureGeometryKHR> geometries{};
			std::vector<uint32_t>                           maxPrimitiveCounts{};

			Desc();
			auto setType(VkAccelerationStructureTypeKHR type) -> Desc &;
			auto setGeometries(const std::vector<VkAccelerationStructureGeometryKHR> &geometrys) -> Desc &;
			auto setMaxPrimitiveCounts(const std::vector<uint32_t> &primitiveCounts) -> Desc &;
			auto setGeometryCount(uint32_t count) -> Desc &;
			auto setFlags(VkBuildAccelerationStructureFlagsKHR flags) -> Desc &;
			auto setDeviceAddress(VkDeviceAddress address) -> Desc &;
		};

		VulkanAccelerationStructure(const uint32_t maxInstanceCount);
		VulkanAccelerationStructure(uint64_t vertexAddress, 
			uint64_t indexAddress, 
			uint32_t vertexStride,
			const std::vector<SubMesh>& subMeshes,
			std::shared_ptr<BatchTask> batch, 
			bool opaque = true);

		virtual ~VulkanAccelerationStructure();

		inline auto &getBuildSizes() const
		{
			return buildSizesInfo;
		}

		virtual auto getBuildScratchSize() const -> uint64_t override
		{
			return buildSizesInfo.buildScratchSize;
		}

		inline auto &getAccelerationStructure() const
		{
			return accelerationStructure;
		}

		virtual auto getDeviceAddress() const -> uint64_t override
		{
			return deviceAddress;
		}

		virtual auto reset()->void override ;

		inline auto getFlags() const
		{
			return flags;
		}

		auto updateTLAS(const mat4 &transform, uint32_t instanceId, uint32_t customInstanceId, uint64_t instanceAddress) -> uint64_t override;

		auto updateBLAS(std::shared_ptr<BatchTask> batch) -> void override;

		auto resetTLAS(uint32_t instanceId) -> void override;

		auto mapHost() -> void * override;

		auto unmap() -> void override;

		auto copyToGPU(const CommandBuffer *cmd, uint32_t instanceSize, uint64_t offset) -> void override;

		auto copyToGPU(const CommandBuffer* cmd, const std::vector<BuildRange>& ranges) -> void override;

		auto build(const CommandBuffer *cmd, uint32_t instanceSize, uint32_t instanceOffset = 0) -> void override;

		inline auto isBuilt() const -> bool override
		{
			return built;
		};

	  protected:
		auto create(const Desc &desc) -> void;

		VulkanBuffer::Ptr                           buffer;
		VkDeviceAddress                             deviceAddress = 0;
		VkBuildAccelerationStructureFlagsKHR        flags         = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
		VkAccelerationStructureBuildSizesInfoKHR    buildSizesInfo{};
		VkAccelerationStructureKHR                  accelerationStructure = nullptr;
		VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo{};
		
	  private:
		bool              built = false;
		VulkanBuffer::Ptr instanceBufferHost;
		VulkanBuffer::Ptr instanceBufferDevice;
		VulkanBuffer::Ptr scratchBuffer;
		//for bottom level..
		std::vector<VkAccelerationStructureBuildRangeInfoKHR> buildRanges;
		std::vector<VkAccelerationStructureGeometryKHR>       geometries;
	};
}        // namespace maple