//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include <cstdint>
#include <functional>
#include <memory>
namespace maple
{
	struct ivec4;
	struct vec4;

	class RenderPass;
	class FrameBuffer;
	class Pipeline;
	class Texture;
	enum class CommandBufferType : uint8_t
	{
		Graphics,
		Compute,
		Raytracing
	};

	class CommandBuffer
	{
	public:
		virtual ~CommandBuffer() = default;

		using Ptr = std::shared_ptr<CommandBuffer>;

		static auto create(CommandBufferType cmdType = CommandBufferType::Graphics)->Ptr;

		virtual auto init(bool primary) -> bool = 0;
		virtual auto unload() -> void = 0;
		virtual auto beginRecording() -> void = 0;
		virtual auto beginRecordingSecondary(RenderPass* renderPass, FrameBuffer* framebuffer) -> void = 0;
		virtual auto endRecording() -> void = 0;
		virtual auto executeSecondary(const CommandBuffer::Ptr& secondayCmdBuffer) -> void = 0;
		virtual auto updateViewport(uint32_t width, uint32_t height) const -> void = 0;
		virtual auto updateViewport(int32_t x, int32_t y, uint32_t width, uint32_t height) const -> void = 0;
		virtual auto bindPipeline(Pipeline* pipeline) -> void = 0;
		virtual auto unbindPipeline() -> void = 0;
		virtual auto endSingleTimeCommands() -> void = 0;
		virtual auto clearAttachments(const  std::shared_ptr<Texture>& attachments, const maple::vec4& value, const maple::ivec4& rect) -> void = 0;
		virtual auto isRecording() const -> bool
		{
			return true;
		};
		virtual auto flush() -> bool
		{
			return true;
		}
		virtual auto addTask(const std::function<void(const CommandBuffer* command)>& task) -> void {};
		virtual auto isSecondary()const  -> bool = 0;
		virtual auto submit() -> void = 0;
	};
}        // namespace maple
