//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#include "RenderDevice.h"
#ifdef MAPLE_VULKAN
#include "Vulkan/VulkanRenderDevice.h"
#endif

#ifdef MAPLE_OPENGL
#include "OpenGL/GLRenderDevice.h"
#endif

#include "FrameBuffer.h"
#include "RenderPass.h"

namespace maple
{
	auto RenderDevice::get() -> std::shared_ptr<RenderDevice>
	{
#ifdef MAPLE_VULKAN
		static auto device = std::make_shared<VulkanRenderDevice>();
#endif // MAPLE_VULKAN

#ifdef MAPLE_OPENGL
		static auto device = std::make_shared<GLRenderDevice>();
#endif
		return device;
	}
} // namespace maple