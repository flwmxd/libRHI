//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////
#include "HashCode.h"
#include "GraphicsContext.h"
#include "Timer.h"

#ifdef MAPLE_VULKAN
#	include "Vulkan/VulkanCommandBuffer.h"
#	include "Vulkan/VulkanComputePipeline.h"
#	include "Vulkan/VulkanContext.h"
#	include "Vulkan/VulkanDescriptorSet.h"
#	include "Vulkan/VulkanFrameBuffer.h"
#	include "Vulkan/VulkanIndexBuffer.h"
#	include "Vulkan/VulkanPipeline.h"
#	include "Vulkan/VulkanRenderPass.h"
#	include "Vulkan/VulkanShader.h"
#	include "Vulkan/VulkanSwapChain.h"
#	include "Vulkan/VulkanUniformBuffer.h"
#	include "Vulkan/VulkanVertexBuffer.h"
#	include "Vulkan/VulkanStorageBuffer.h"
#	include "Vulkan/Raytracing/VulkanRaytracingPipeline.h"
#	include "Vulkan/VKImGuiRenderer.h"
#endif        // MAPLE_VULKAN

#ifdef MAPLE_OPENGL
#	include "RHI/ImGui/GLImGuiRenderer.h"
#	include "RHI/OpenGL/GLCommandBuffer.h"
#	include "RHI/OpenGL/GLContext.h"
#	include "RHI/OpenGL/GLDescriptorSet.h"
#	include "RHI/OpenGL/GLFrameBuffer.h"
#	include "RHI/OpenGL/GLIndexBuffer.h"
#	include "RHI/OpenGL/GLPipeline.h"
#	include "RHI/OpenGL/GLRenderPass.h"
#	include "RHI/OpenGL/GLShader.h"
#	include "RHI/OpenGL/GLSwapChain.h"
#	include "RHI/OpenGL/GLUniformBuffer.h"
#	include "RHI/OpenGL/GLVertexBuffer.h"
#	include "RHI/OpenGL/GLStorageBuffer.h"
#endif

namespace maple
{
	auto GraphicsContext::get() -> std::shared_ptr<GraphicsContext>
	{

#ifdef MAPLE_VULKAN
		static auto context = std::make_shared<VulkanContext>();
#endif        // MAPLE_VULKAN
#ifdef MAPLE_OPENGL
		static auto context = std::make_shared<GLContext>();
#endif        // MAPLE_OPENGL
		return context;
	}

	auto GraphicsContext::clearUnused() -> void
	{
		auto current = Timer::get().currentTimestamp();

		for (auto iter = pipelineCache.begin(); iter != pipelineCache.end();)
		{
			if (iter->second.asset.use_count() == 1 && (current - iter->second.lastTimestamp) > 12000)
			{
				LOGI("Pipeline clear");
				iter = pipelineCache.erase(iter);
				continue;
			}
			iter++;
		}

		for (auto iter = frameBufferCache.begin(); iter != frameBufferCache.end();)
		{
			if (iter->second.asset.use_count() == 1 && (current - iter->second.lastTimestamp) > 12000)
			{
				LOGI("FrameBuffer clear");
				iter = frameBufferCache.erase(iter);
				continue;
			}
			iter++;
		}
	}

	auto SwapChain::create(uint32_t width, uint32_t height) -> std::shared_ptr<SwapChain>
	{
#ifdef MAPLE_VULKAN
		return std::make_shared<VulkanSwapChain>(width, height);
#endif
#ifdef MAPLE_OPENGL
		return std::make_shared<GLSwapChain>(width, height);
#endif
	}

	auto Shader::create(const std::string& filePath, const VariableArraySize& size) -> std::shared_ptr<Shader>
	{
#ifdef MAPLE_VULKAN
		return std::make_shared<VulkanShader>(filePath, size);
#endif

#ifdef MAPLE_OPENGL
		return io::emplace<GLShader>(filePath, filePath);
#endif
	}

	auto Shader::create(const std::vector<uint32_t>& vertData, const std::vector<uint32_t>& fragData, const std::unordered_set<std::string>& dynamicUniforms) -> std::shared_ptr<Shader>
	{
#ifdef MAPLE_VULKAN
		return std::make_shared<VulkanShader>(vertData, fragData, dynamicUniforms);
#endif
#ifdef MAPLE_OPENGL
		return std::make_shared<GLShader>(vertData, fragData);
#endif
	}

	auto Shader::createRaw(const std::vector<uint32_t>& vertData, const std::vector<uint32_t>& fragData, const std::unordered_set<std::string>& dynamicUniforms) ->Shader*
	{
		return new VulkanShader(vertData, fragData, dynamicUniforms);
	}

	auto FrameBuffer::create(const FrameBufferInfo& desc) -> std::shared_ptr<FrameBuffer>
	{
		size_t hash = 0;
		hash::hashCode(hash, desc.attachments.size(), desc.width, desc.height, desc.layer, desc.renderPass, desc.screenFBO);

		for (uint32_t i = 0; i < desc.attachments.size(); i++)
		{
			hash::hashCode(hash, desc.attachments[i]);
#ifdef MAPLE_VULKAN
			VkDescriptorImageInfo* imageHandle = (VkDescriptorImageInfo*)(desc.attachments[i]->getDescriptorInfo());
			hash::hashCode(hash, imageHandle->imageLayout, imageHandle->imageView, imageHandle->sampler);
#endif
		}
		auto& frameBufferCache = GraphicsContext::get()->getFrameBufferCache();
		auto  found = frameBufferCache.find(hash);
		if (found != frameBufferCache.end() && found->second.asset)
		{
			found->second.lastTimestamp = Timer::get().currentTimestamp();
			return found->second.asset;
		}
#ifdef MAPLE_VULKAN

		std::shared_ptr<FrameBuffer> fb = std::make_shared<VulkanFrameBuffer>(desc);
		return frameBufferCache.emplace(std::piecewise_construct, std::forward_as_tuple(hash), std::forward_as_tuple(fb, Timer::get().currentTimestamp())).first->second.asset;
#endif
#ifdef MAPLE_OPENGL
		std::shared_ptr<FrameBuffer> fb = std::make_shared<GLFrameBuffer>(desc);
		return frameBufferCache.emplace(std::piecewise_construct, std::forward_as_tuple(hash), std::forward_as_tuple(fb, Timer::get().currentTimestamp())).first->second.asset;
#endif
	}

	auto DescriptorSet::create(const DescriptorInfo& desc) -> std::shared_ptr<DescriptorSet>
	{
#ifdef MAPLE_VULKAN
		return std::make_shared<VulkanDescriptorSet>(desc);
#endif
#ifdef MAPLE_OPENGL
		return std::make_shared<GLDescriptorSet>(desc);
#endif
	}

	auto CommandBuffer::create(CommandBufferType cmdType) -> std::shared_ptr<CommandBuffer>
	{
#ifdef MAPLE_VULKAN
		return std::make_shared<VulkanCommandBuffer>(cmdType);
#endif
#ifdef MAPLE_OPENGL
		return std::make_shared<GLCommandBuffer>();
#endif
	}

	auto IndexBuffer::create(const uint16_t* data, uint32_t count, BufferUsage bufferUsage, bool gpuOnly) -> std::shared_ptr<IndexBuffer>
	{
#ifdef MAPLE_VULKAN
		return std::make_shared<VulkanIndexBuffer>(data, count, bufferUsage, gpuOnly);
#endif
#ifdef MAPLE_OPENGL
		return std::make_shared<GLIndexBuffer>(data, count, bufferUsage);
#endif
	}
	auto IndexBuffer::create(const uint32_t* data, uint32_t count, BufferUsage bufferUsage, bool gpuOnly) -> std::shared_ptr<IndexBuffer>
	{
#ifdef MAPLE_VULKAN
		return std::make_shared<VulkanIndexBuffer>(data, count, bufferUsage, gpuOnly);
#endif
#ifdef MAPLE_OPENGL
		return std::make_shared<GLIndexBuffer>(data, count, bufferUsage);
#endif
	}

	auto IndexBuffer::createRaw(const uint16_t* data, uint32_t count, BufferUsage bufferUsage, bool gpuOnly) -> IndexBuffer*
	{
#ifdef MAPLE_VULKAN
		return new VulkanIndexBuffer(data, count, bufferUsage, gpuOnly);
#endif
#ifdef MAPLE_OPENGL
		return new GLIndexBuffer(data, count, bufferUsage);
#endif
	}
	auto IndexBuffer::createRaw(const uint32_t* data, uint32_t count, BufferUsage bufferUsage, bool gpuOnly) -> IndexBuffer*
	{
#ifdef MAPLE_VULKAN
		return new VulkanIndexBuffer(data, count, bufferUsage, gpuOnly);
#endif
#ifdef MAPLE_OPENGL
		return new GLIndexBuffer(data, count, bufferUsage);
#endif
	}

	auto Pipeline::get(const PipelineInfo& desc) -> std::shared_ptr<Pipeline>
	{
		size_t hash = 0;

		hash::hashCode(hash, desc.shader, desc.cullMode, desc.depthBiasEnabled, desc.drawType, desc.polygonMode, desc.transparencyEnabled, desc.depthWriteEnable);
		hash::hashCode(hash, desc.stencilMask, desc.stencilFunc, desc.stencilFail, desc.stencilDepthFail, desc.stencilDepthPass, desc.depthTest);

		for (auto texture : desc.colorTargets)
		{
			if (texture)
			{
				hash::hashCode(hash, texture);
				hash::hashCode(hash, texture->getWidth(), texture->getHeight());
				hash::hashCode(hash, texture->getHandle());
				hash::hashCode(hash, texture->getFormat());
#ifdef MAPLE_VULKAN
				VkDescriptorImageInfo* imageHandle = (VkDescriptorImageInfo*)(texture->getDescriptorInfo());
				hash::hashCode(hash, imageHandle->imageLayout, imageHandle->imageView, imageHandle->sampler);
#endif
			}
		}

		hash::hashCode(hash, desc.clearTargets);
		hash::hashCode(hash, desc.depthTarget);
		hash::hashCode(hash, desc.depthArrayTarget);
		hash::hashCode(hash, desc.swapChainTarget);
		//hash::hashCode(hash, desc.groupCountX, desc.groupCountY, desc.groupCountZ);

		if (desc.swapChainTarget)
		{
			auto texture = GraphicsContext::get()->getSwapChain()->getCurrentImage();
			if (texture)
			{
				hash::hashCode(hash, texture);
				hash::hashCode(hash, texture->getWidth(), texture->getHeight());
				hash::hashCode(hash, texture->getHandle());
				hash::hashCode(hash, texture->getFormat());
#ifdef MAPLE_VULKAN
				VkDescriptorImageInfo* imageHandle = (VkDescriptorImageInfo*)(texture->getDescriptorInfo());
				hash::hashCode(hash, imageHandle->imageLayout, imageHandle->imageView, imageHandle->sampler);
#endif
			}
		}
		auto& pipelineCache = GraphicsContext::get()->getPipelineCache();
		auto  found = pipelineCache.find(hash);

		if (found != pipelineCache.end() && found->second.asset)
		{
			found->second.lastTimestamp = Timer::get().currentTimestamp();
			return found->second.asset;
		}

#ifdef MAPLE_OPENGL
		std::shared_ptr<Pipeline> pipeline = std::make_shared<GLPipeline>(desc);
		return pipelineCache.emplace(std::piecewise_construct, std::forward_as_tuple(hash), std::forward_as_tuple(pipeline, Application::getTimer().currentTimestamp())).first->second.asset;
#endif        // MAPLE_OPENGL

#ifdef MAPLE_VULKAN
		std::shared_ptr<Pipeline> pipeline;
		if (desc.shader->isComputeShader())
		{
			pipeline = std::make_shared<VulkanComputePipeline>(desc);
		}
		else if (desc.shader->isRaytracingShader())
		{
			pipeline = std::make_shared<VulkanRaytracingPipeline>(desc);
		}
		else
		{
			pipeline = std::make_shared<VulkanPipeline>(desc);
		}
		return pipelineCache.emplace(std::piecewise_construct, std::forward_as_tuple(hash), std::forward_as_tuple(pipeline, Timer::get().currentTimestamp())).first->second.asset;
#endif        // MAPLE_OPENGL
	}

	auto RenderPass::create(const RenderPassInfo& desc) -> Ptr
	{
#ifdef MAPLE_VULKAN
		struct RenderPassAsset
		{
			RenderPass::Ptr renderPass;
			int64_t         timeSinceLastAccessed;
		};
		static std::unordered_map<uint64_t, RenderPassAsset> renderPassCache;

		uint64_t hash = 0;
		hash::hashCode(hash, desc.attachments.size(), desc.clear);

		for (uint32_t i = 0; i < desc.attachments.size(); i++)
		{
			hash::hashCode(hash, desc.attachments[i]);
		}

		auto found = renderPassCache.find(hash);
		if (found != renderPassCache.end() && found->second.renderPass)
		{
			found->second.timeSinceLastAccessed = Timer::get().currentTimestamp();
			return found->second.renderPass;
		}

		return renderPassCache.emplace(
			hash, RenderPassAsset{ std::make_shared<VulkanRenderPass>(desc), Timer::get().currentTimestamp() })
			.first->second.renderPass;
#endif
#ifdef MAPLE_OPENGL
		return std::make_shared<GLRenderPass>(desc);
#endif
	}

	auto UniformBuffer::create() -> std::shared_ptr<UniformBuffer>
	{
#ifdef MAPLE_VULKAN
		return std::make_shared<VulkanUniformBuffer>();
#endif
#ifdef MAPLE_OPENGL
		return std::make_shared<GLUniformBuffer>();
#endif
	}

	auto UniformBuffer::create(uint32_t size, const void* data) -> std::shared_ptr<UniformBuffer>
	{
#ifdef MAPLE_VULKAN
		auto buffer = std::make_shared<VulkanUniformBuffer>(size, data);
#endif
#ifdef MAPLE_OPENGL
		auto buffer = std::make_shared<GLUniformBuffer>();
#endif
		//buffer->setData(size, data);
		return buffer;
	}

	auto VertexBuffer::create(const BufferUsage& usage) -> std::shared_ptr<VertexBuffer>
	{
#ifdef MAPLE_VULKAN
		return std::make_shared<VulkanVertexBuffer>(usage);
#endif
#ifdef MAPLE_OPENGL
		return std::make_shared<GLVertexBuffer>(usage);
#endif
	}

	auto VertexBuffer::create(const void* data, uint32_t size, bool gpuOnly) -> std::shared_ptr<VertexBuffer>
	{
#ifdef MAPLE_VULKAN
		return std::make_shared<VulkanVertexBuffer>(data, size, gpuOnly);
#endif
#ifdef MAPLE_OPENGL
		return std::make_shared<GLVertexBuffer>(data, size);
#endif
	}

	auto VertexBuffer::createRaw(const BufferUsage& usage)->VertexBuffer*
	{
		return new VulkanVertexBuffer(usage);
	}

	auto VertexBuffer::createRaw(const void* data, uint32_t size, bool gpuOnly)->VertexBuffer*
	{
		return new VulkanVertexBuffer(data, size, gpuOnly);
	}

	auto StorageBuffer::create(const BufferOptions& options) -> std::shared_ptr<StorageBuffer>
	{
#ifdef MAPLE_VULKAN
		return std::make_shared<VulkanStorageBuffer>(options);
#endif
#ifdef MAPLE_OPENGL
		return std::make_shared<GLStorageBuffer>();
#endif
	}

	auto StorageBuffer::create(uint32_t size, uint32_t flags, const BufferOptions& options) -> std::shared_ptr<StorageBuffer>
	{
#ifdef MAPLE_VULKAN
		return std::make_shared<VulkanStorageBuffer>(size, flags, options);
#endif
#ifdef MAPLE_OPENGL
		return std::make_shared<GLStorageBuffer>(size, nullptr);
#endif
	}

	auto StorageBuffer::create(uint32_t size, const void* data, const BufferOptions& options) -> std::shared_ptr<StorageBuffer>
	{
#ifdef MAPLE_VULKAN
		return std::make_shared<VulkanStorageBuffer>(size, data, options);
#endif
#ifdef MAPLE_OPENGL
		return std::make_shared<GLStorageBuffer>(size, data);
#endif
	}

	auto ImGuiRenderer::create(uint32_t width, uint32_t height, bool clearScreen)->std::shared_ptr<ImGuiRenderer>
	{
		return std::make_shared<VKImGuiRenderer>(width, height, clearScreen);
	}
}        // namespace maple
