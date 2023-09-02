//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "BufferLayout.h"
#include "Definitions.h"
#include "Shader.h"
#include <array>
#include <memory>

namespace maple
{
	class ShaderBindingTable;

	class DescriptorSet;

	class  Pipeline
	{
	public:
		using Ptr = std::shared_ptr<Pipeline>;

		static auto get(const PipelineInfo& pipelineDesc)->std::shared_ptr<Pipeline>;

		virtual ~Pipeline() = default;

		virtual auto getWidth()->uint32_t = 0;
		virtual auto getHeight()->uint32_t = 0;
		virtual auto getShader() const->std::shared_ptr<Shader> = 0;
		virtual auto bind(const CommandBuffer* commandBuffer, uint32_t layer = 0, int32_t cubeFace = -1, int32_t mipMapLevel = 0)->FrameBuffer* = 0;
		virtual auto bind(const CommandBuffer* commandBuffer, const ivec4& viewport)->FrameBuffer* = 0;

		virtual auto end(const CommandBuffer* commandBuffer) -> void = 0;
		virtual auto clearRenderTargets(const CommandBuffer* commandBuffer) -> void {};
		virtual auto traceRays(const CommandBuffer* commandBuffer, uint32_t width, uint32_t height, uint32_t depth) -> void {};
		virtual auto dispatchIndirect(const CommandBuffer* cmdBuffer, const StorageBuffer* ssbo, uint64_t offset = 0) -> void {};

		virtual auto drawIndexedIndirect(const CommandBuffer* cmdBuffer, const StorageBuffer* ssbo, uint32_t drawCount, uint32_t stride = 0, uint64_t offset = 0) -> void {};

		virtual auto drawIndexed(const CommandBuffer* cmdBuffer,
			uint32_t             indexCount,
			uint32_t             instanceCount,
			uint32_t             firstIndex,
			int32_t              vertexOffset,
			uint32_t             firstInstance) -> void = 0;

		virtual auto bufferBarrier(const CommandBuffer* commandBuffer,
			const std::vector<std::shared_ptr<StorageBuffer>>& buffers, bool read) -> void = 0;

		inline auto& getDescription() const
		{
			return description;
		}

	protected:
		PipelineInfo description;
	};
}        // namespace maple
