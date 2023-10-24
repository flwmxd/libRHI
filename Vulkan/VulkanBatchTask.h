//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../BatchTask.h"
#include "VulkanHelper.h"
namespace maple
{
	class VulkanAccelerationStructure;

	class VulkanBatchTask : public BatchTask
	{
	  public:
		virtual auto execute(const CommandBuffer * cmd) -> void override;

		auto buildBlas(VulkanAccelerationStructure* accelerationStructure,
			const std::vector<VkAccelerationStructureGeometryKHR>& geometries,
			const std::vector<VkAccelerationStructureBuildRangeInfoKHR> buildRanges) -> void;

	  private:
		struct BLASBuildRequest
		{
			VulkanAccelerationStructure *                         accelerationStructure;
			std::vector<VkAccelerationStructureGeometryKHR>       geometries;
			std::vector<VkAccelerationStructureBuildRangeInfoKHR> buildRanges;
		};

		std::vector<BLASBuildRequest> requests;

		std::shared_ptr<VulkanBuffer> scratchBuffer;
	};
};        // namespace maple