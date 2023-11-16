//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#pragma once

#include "BatchTask.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include <memory>

namespace maple
{
	class StorageBuffer;
	class CommandBuffer;

	struct SubMesh 
	{
		uint32_t indexCount;
		uint32_t indexOffset;
		int32_t numberOfVertex;
	};

	struct BuildRange 
	{
		int32_t count;
		int32_t instanceOffset;
	};

	class  AccelerationStructure
	{
	  public:
		using Ptr = std::shared_ptr<AccelerationStructure>;

		static auto createTopLevel(const uint32_t maxInstanceCount) -> Ptr;

		static auto createBottomLevel(const VertexBuffer::Ptr &vertexBuffer, const IndexBuffer::Ptr &indexBuffer, uint32_t vertexStride, const std::vector<SubMesh>& subMeshes, BatchTask::Ptr tasks) -> Ptr;

		static auto createBottomLevel(const VertexBuffer * vertexBuffer, const IndexBuffer* indexBuffer, uint32_t vertexStride, const std::vector<SubMesh> & subMeshes, BatchTask::Ptr tasks)->Ptr;

		virtual auto getBuildScratchSize() const -> uint64_t = 0;

		virtual auto updateTLAS(const mat4 &transform, uint32_t instanceId, uint32_t customInstanceId, uint64_t instanceAddress) -> uint64_t = 0;

		virtual auto updateBLAS(std::shared_ptr<BatchTask> batch) -> void = 0;

		virtual auto resetTLAS(uint32_t instanceId) -> void = 0;
		
		virtual auto reset()->void = 0;

		virtual auto getDeviceAddress() const -> uint64_t = 0;

		virtual auto mapHost() -> void * = 0;

		virtual auto unmap() -> void = 0;

		virtual auto copyToGPU(const CommandBuffer *cmd, uint32_t instanceSize, uint64_t offset) -> void = 0;

		virtual auto copyToGPU(const CommandBuffer* cmd, const std::vector<BuildRange> & ranges) -> void = 0;

		virtual auto build(const CommandBuffer *cmd, uint32_t instanceSize, uint32_t instanceOffset = 0) -> void = 0;

		virtual auto isBuilt() const -> bool = 0;
	};

	class NullAccelerationStructure : public AccelerationStructure
	{
	  public:
		virtual auto getBuildScratchSize() const -> uint64_t
		{
			return 0;
		}

		virtual auto reset()->void override {};

		virtual auto updateTLAS(const mat4 &transform, uint32_t instanceId, uint32_t customInstanceId, uint64_t instanceAddress) -> uint64_t
		{
			return 0;
		}

		virtual auto resetTLAS(uint32_t instanceId) -> void {}

		virtual auto getDeviceAddress() const -> uint64_t
		{
			return 0;
		}

		virtual auto mapHost() -> void *
		{
			return nullptr;
		}

		virtual auto unmap() -> void
		{
		}

		virtual auto copyToGPU(const CommandBuffer *cmd, uint32_t instanceSize, uint64_t offset) -> void
		{}

		virtual auto copyToGPU(const CommandBuffer* cmd, const std::vector<BuildRange>& ranges) -> void override {};

		virtual auto build(const CommandBuffer *cmd, uint32_t instanceSize, uint32_t instanceOffset = 0) -> void{};

		virtual auto isBuilt() const -> bool override
		{
			return true;
		};
	};        // namespace maple

}        // namespace maple