//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../CommandBuffer.h"
#include "VulkanHelper.h"

namespace maple
{
	class VulkanFence;

	enum class CommandBufferState : uint8_t
	{
		Idle,
		Recording,
		Ended,
		Submitted
	};

	class VulkanCommandBuffer : public CommandBuffer
	{
	public:
		VulkanCommandBuffer(CommandBufferType cmdBufferType = CommandBufferType::Graphics);
		VulkanCommandBuffer(VkCommandBuffer commandBuffer, CommandBufferType cmdBufferType = CommandBufferType::Graphics);
		~VulkanCommandBuffer();

		auto init(bool primary) -> bool override;
		auto init(bool primary, VkCommandPool commandPool) -> bool;
		auto unload() -> void override;
		auto beginRecording() -> void override;
		auto beginRecordingSecondary(RenderPass* renderPass, FrameBuffer* framebuffer) -> void override;
		auto endRecording() -> void override;
		//primary executes secondary
		auto executeSecondary(const CommandBuffer::Ptr& secondaryCmdBuffer) -> void override;
		auto updateViewport(uint32_t width, uint32_t height) const -> void override;
		auto updateViewport(int32_t x, int32_t y, uint32_t width, uint32_t height) const -> void override;
		auto clearAttachments(const std::shared_ptr<Texture>& attachments, const maple::vec4& value, const maple::ivec4& rect) -> void override;

		auto bindPipeline(Pipeline* pipeline) -> void override;
		auto unbindPipeline() -> void override;
		auto flush() -> bool override;
		auto endSingleTimeCommands() -> void override;

		auto submit() -> void override;

		inline auto isRecording() const -> bool override
		{
			return state == CommandBufferState::Recording;
		}

		inline auto getState() const
		{
			return state;
		}

		auto wait() -> void;
		auto reset() -> void;

		auto executeInternal(
			const std::vector<VkPipelineStageFlags>& flags,
			const std::vector<VkSemaphore>& waitSemaphores,
			const std::vector<VkSemaphore>& signalSemaphores,
			bool                                     waitFence) -> void;

		inline auto getCommandBuffer() const
		{
			return commandBuffer;
		}

		inline auto getCommandBuffeType() const
		{
			return cmdBufferType;
		}

		auto addTask(const std::function<void(const CommandBuffer*)>& task) -> void override;

		inline auto& getSemaphore() const
		{
			return rendererSemaphore;
		}

		auto isSecondary() const -> bool override { return !primary; }
	private:
		VkCommandBuffer commandBuffer = nullptr;
		VkCommandPool   commandPool = nullptr;
		bool            primary;

		CommandBufferState state = CommandBufferState::Idle;

		Pipeline* boundPipeline = nullptr;
		RenderPass* boundRenderPass = nullptr;

		CommandBufferType cmdBufferType = CommandBufferType::Graphics;

		std::vector<std::function<void(const CommandBuffer*)>> tasks;
		std::shared_ptr<VulkanFence>                            fence;
		VkSemaphore                                             rendererSemaphore = VK_NULL_HANDLE;
		std::vector< CommandBuffer::Ptr> secondaryCommands;
		std::unique_ptr<VulkanFence> updateFence;
	};
};        // namespace maple
