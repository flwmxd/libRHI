//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#pragma once

#include <cstdint>
#include <memory>
#include <functional>

namespace maple
{
	class CommandBuffer;

	class ImGuiRenderer
	{
	public:
		using Ptr = std::shared_ptr<ImGuiRenderer>;

		static auto create(uint32_t width, uint32_t height, bool clearScreen)->std::shared_ptr<ImGuiRenderer>;

		virtual ~ImGuiRenderer() = default;
		virtual auto init() -> void = 0;
		virtual auto newFrame() -> void = 0;
		virtual auto render(const std::function<void()>& userCallback) -> void = 0;
		virtual auto onResize(uint32_t width, uint32_t height) -> void = 0;
		virtual auto rebuildFontTexture() -> void = 0;
		virtual auto clear() -> void {};
	};
}        // namespace maple
