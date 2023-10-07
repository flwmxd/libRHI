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

	class  AccelerationStructure
	{
	  public:
		using Ptr = std::shared_ptr<AccelerationStructure>;

		static auto createTopLevel(const uint32_t maxInstanceCount) -> Ptr;

		static auto createBottomLevel(const VertexBuffer::Ptr &vertexBuffer, const IndexBuffer::Ptr &indexBuffer, uint32_t vertexCount, uint32_t vertexStride, BatchTask::Ptr tasks) -> Ptr;

		static auto createBottomLevel(const VertexBuffer * vertexBuffer, const IndexBuffer* indexBuffer, uint32_t vertexCount, uint32_t vertexStride, BatchTask::Ptr tasks)->Ptr;

		virtual auto getBuildScratchSize() const -> uint64_t = 0;

		virtual auto updateTLAS(const mat4 &transform, uint32_t instanceId, uint64_t instanceAddress) -> uint64_t = 0;

		virtual auto getDeviceAddress() const -> uint64_t = 0;

		virtual auto mapHost() -> void * = 0;

		virtual auto unmap() -> void = 0;

		virtual auto copyToGPU(const CommandBuffer *cmd, uint32_t instanceSize, uint64_t offset) -> void = 0;

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

		virtual auto updateTLAS(const mat4 &transform, uint32_t instanceId, uint64_t instanceAddress) -> uint64_t
		{
			return 0;
		}

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

		virtual auto build(const CommandBuffer *cmd, uint32_t instanceSize, uint32_t instanceOffset = 0) -> void{};

		virtual auto isBuilt() const -> bool override
		{
			return true;
		};
	};        // namespace maple

}        // namespace maple