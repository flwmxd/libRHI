//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Definitions.h"
#include <memory>

namespace maple
{
	class  RenderPass
	{
	public:
		using Ptr = std::shared_ptr<RenderPass>;

		virtual ~RenderPass() = default;
		static auto create(const RenderPassInfo& renderPassDesc)->Ptr;

		virtual auto getAttachmentCount() const->int32_t = 0;
		virtual auto beginRenderPass(const CommandBuffer* commandBuffer, const float* clearColor/*float[4]*/, FrameBuffer* frame, SubPassContents contents, uint32_t width, uint32_t height, int32_t cubeFace, int32_t mipMapLevel) const -> void = 0;
		virtual auto beginRenderPass(const CommandBuffer* commandBuffer, const float* clearColor/*float[4]*/, FrameBuffer* frame, SubPassContents contents, uint32_t width, uint32_t height, const int32_t* viewport) const -> void = 0;
		virtual auto endRenderPass(const CommandBuffer* commandBuffer) -> void = 0;
	};
}        // namespace maple
