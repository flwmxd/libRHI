//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#pragma once

#include "AccessFlags.h"
#include "Definitions.h"
#include <assert.h>
#include <memory>

namespace maple
{
	class Texture;
	class CommandBuffer;
	class Pipeline;
	class StorageBuffer;

	class  RenderDevice
	{
	public:
		RenderDevice() = default;
		virtual ~RenderDevice() = default;
		virtual auto init() -> void = 0;
		virtual auto begin() -> void = 0;
		virtual auto onResize(uint32_t width, uint32_t height) -> void = 0;
		virtual auto drawSplashScreen(const std::shared_ptr<Texture>& texture) -> void {};
		virtual auto clear(const std::shared_ptr<StorageBuffer>& buffer, int32_t value, const CommandBuffer* command) -> void {};

		virtual auto setDepthTestingInternal(bool enabled, StencilType func = StencilType::Less) -> void {};
		virtual auto setStencilTestInternal(bool enabled) -> void {};
		virtual auto setStencilMaskInternal(uint32_t mask) -> void {};
		virtual auto setStencilOpInternal(StencilType fail, StencilType zfail, StencilType zpass) -> void {};
		virtual auto setStencilFunctionInternal(StencilType type, uint32_t ref, uint32_t mask) -> void {};

		virtual auto presentInternal() -> void {};
		virtual auto presentInternal(const CommandBuffer* commandBuffer) -> void {};

		virtual auto dispatch(const CommandBuffer* commandBuffer, uint32_t x, uint32_t y, uint32_t z) -> void {};
		virtual auto drawInstanced(const CommandBuffer* commandBuffer, uint32_t verticesCount, uint32_t instanceCount, int32_t startInstance = 0, int32_t startVertex = 0) const -> void {};
		virtual auto drawIndexedIndirect(const CommandBuffer* commandBuffer, const StorageBuffer* indirectBuffer, uint64_t offset, uint32_t count, uint32_t stride) -> void {};
		virtual auto drawIndirect(const CommandBuffer* commandBuffer, const StorageBuffer* indirectBuffer, uint64_t offset, uint32_t count, uint32_t stride) -> void {};

		virtual auto memoryBarrier(const CommandBuffer* commandBuffer, uint32_t flag) -> void {};
		virtual auto memoryBarrier(const CommandBuffer* commandBuffer, ShaderType fromStage, ShaderType toStage, AccessFlags from, AccessFlags to) -> void {};
		virtual auto bufferMemoryBarrier(const CommandBuffer* commandBuffer, ShaderType fromStage, ShaderType toStage, const std::vector<BufferBarrier>& barries) const -> void {};
		virtual auto imageBarrier(const CommandBuffer* commandBuffer, const ImageMemoryBarrier& barriers) -> void {};

		virtual auto drawArraysInternal(const CommandBuffer* commandBuffer, DrawType type, uint32_t count, uint32_t start = 0) const -> void {};
		virtual auto drawIndexedInternal(const CommandBuffer* commandBuffer, DrawType type, uint32_t count, uint32_t start = 0, uint32_t vertexOffset = 0) const -> void {};

		virtual auto drawInternal(const CommandBuffer* commandBuffer, DrawType type, uint32_t count, DataType dataType = DataType::UnsignedInt, const void* indices = nullptr) const -> void {};
		virtual auto bindDescriptorSets(Pipeline* pipeline, const CommandBuffer* commandBuffer, const std::vector<std::shared_ptr<DescriptorSet>>& descriptorSets) -> void {};
		virtual auto bindDescriptorSet(Pipeline* pipeline, const CommandBuffer* commandBuffer, int32_t index, const std::shared_ptr<DescriptorSet>& descriptorSet) -> void {};
		virtual auto bindDescriptorSetsInternal(Pipeline* pipeline, const CommandBuffer* commandBuffer, uint32_t dynamicOffset, const std::vector<std::shared_ptr<DescriptorSet>>& descriptorSets) -> void {};


		virtual auto clearRenderTarget(const std::shared_ptr<Texture>& texture, const CommandBuffer* commandBuffer, const vec4& clearColor = {0,0,0,0}) -> void {};
		virtual auto clearRenderTarget(const std::shared_ptr<Texture>& texture, const CommandBuffer* commandBuffer, const uint32_t* clearColor) -> void {};

		virtual auto clearInternal(uint32_t bufferMask) -> void {};

		virtual auto copyBuffer(const CommandBuffer* commandBuffer,
			const std::shared_ptr<StorageBuffer>& from,
			const std::shared_ptr<StorageBuffer>& to, uint32_t size, uint32_t dstOffset = 0, uint32_t srcOffset = 0, bool barrier = true) const -> void = 0;

		static auto get()->std::shared_ptr<RenderDevice>;
	};
};           // namespace maple
