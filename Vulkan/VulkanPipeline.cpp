//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////

#include "VulkanPipeline.h"
#include "../Console.h"
#include "VulkanCommandBuffer.h"
#include "VulkanContext.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDevice.h"
#include "VulkanFrameBuffer.h"
#include "VulkanRenderDevice.h"
#include "VulkanRenderPass.h"
#include "VulkanShader.h"
#include "VulkanStorageBuffer.h"
#include "VulkanSwapChain.h"
#include "VulkanTexture.h"
#include <memory>

namespace maple
{
	namespace
	{

		inline auto convertBlend(BlendMode mode)
		{
			switch(mode) {
			case maple::BlendMode::None: MAPLE_ASSERT(false, "Unsupported"); break;
			case maple::BlendMode::Zero: return VK_BLEND_FACTOR_ZERO;
			case maple::BlendMode::One: return VK_BLEND_FACTOR_ONE;
			case maple::BlendMode::SrcColor: return VK_BLEND_FACTOR_SRC_COLOR;
			case maple::BlendMode::OneMinusSrcColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
			case maple::BlendMode::SrcAlpha: return VK_BLEND_FACTOR_SRC_ALPHA;
			case maple::BlendMode::OneMinusSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			case maple::BlendMode::DstAlpha: return VK_BLEND_FACTOR_DST_ALPHA;
			case maple::BlendMode::OneMinusDstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
			case maple::BlendMode::DstColor: return VK_BLEND_FACTOR_DST_COLOR;
			case maple::BlendMode::OneMinusDstColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
			case maple::BlendMode::SrcAlphaSaturate: return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
			default: break;
			}
			MAPLE_ASSERT(false, "Unsupported");
			return VK_BLEND_FACTOR_ZERO;
		}

		inline auto convertCompareOp(StencilType type)
		{
			switch(type) {
			case maple::StencilType::Equal: return VK_COMPARE_OP_EQUAL;
			case maple::StencilType::Notequal: return VK_COMPARE_OP_NOT_EQUAL;
			case maple::StencilType::Always: return VK_COMPARE_OP_ALWAYS;
			case StencilType::Never: return VK_COMPARE_OP_NEVER;
			case StencilType::Less: return VK_COMPARE_OP_LESS;
			case StencilType::LessOrEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
			case StencilType::Greater: return VK_COMPARE_OP_GREATER;
			case StencilType::GreaterOrEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
			case maple::StencilType::Keep:
			case maple::StencilType::Replace:
			case maple::StencilType::Zero: throw std::logic_error("Compare Op should not be Keep/Replace/Zero");
			}

			return VK_COMPARE_OP_LESS_OR_EQUAL;
		}

		inline auto convertStencilOp(StencilType type)
		{
			switch(type) {
			case maple::StencilType::Keep: return VK_STENCIL_OP_KEEP;
			case maple::StencilType::Replace: return VK_STENCIL_OP_REPLACE;
			case maple::StencilType::Zero: return VK_STENCIL_OP_ZERO;
			}
			throw std::logic_error("Compare Op should be Keep/Replace/Zero");
		}

		inline auto createDepthStencil(VkPipelineDepthStencilStateCreateInfo &ds, const PipelineInfo &info) -> void
		{
			ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			ds.pNext = NULL;
			ds.depthTestEnable = info.depthTest ? VK_TRUE : VK_FALSE;
			ds.depthWriteEnable = info.depthWriteEnable ? VK_TRUE : VK_FALSE;
			ds.depthCompareOp = convertCompareOp(info.depthFunc);
			ds.depthBoundsTestEnable = VK_FALSE;
			ds.stencilTestEnable = info.stencilTest ? VK_TRUE : VK_FALSE;

			ds.back.failOp = convertStencilOp(info.stencilFail);
			ds.back.passOp = convertStencilOp(info.stencilDepthPass);
			ds.back.compareOp = convertCompareOp(info.stencilFunc);
			ds.back.compareMask = 0xFF;
			ds.back.reference = 1;
			ds.back.depthFailOp = convertStencilOp(info.stencilDepthFail);
			ds.back.writeMask = info.stencilMask;

			ds.minDepthBounds = 0;
			ds.maxDepthBounds = 0;
			ds.front = ds.back;
		}

		inline auto createMultisample(VkPipelineMultisampleStateCreateInfo &ms) -> void
		{
			ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			ms.pNext = NULL;
			ms.pSampleMask = NULL;
			ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			ms.sampleShadingEnable = VK_FALSE;
			ms.alphaToCoverageEnable = VK_FALSE;
			ms.alphaToOneEnable = VK_FALSE;
			ms.minSampleShading = 0.0;
		}

		inline auto createVertexLayout(VkVertexInputBindingDescription &vertexBindingDescription, VkPipelineVertexInputStateCreateInfo &vi,
		                               std::shared_ptr<VulkanShader> vkShader) -> void
		{
			vertexBindingDescription.binding = 0;
			vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			vertexBindingDescription.stride = vkShader->getVertexInputStride();

			auto &vertexInputAttributeDescription = vkShader->getVertexInputAttributeDescription();

			vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vi.pNext = NULL;
			vi.vertexBindingDescriptionCount = vertexBindingDescription.stride != 0 ? 1 : 0;
			vi.pVertexBindingDescriptions = vertexBindingDescription.stride != 0 ? &vertexBindingDescription : nullptr;
			vi.vertexAttributeDescriptionCount = uint32_t(vertexInputAttributeDescription.size());
			vi.pVertexAttributeDescriptions = vertexInputAttributeDescription.data();
		}

		inline auto createRasterization(VkPipelineInputAssemblyStateCreateInfo &inputAssemblyCI, VkPipelineRasterizationStateCreateInfo &rs,
		                                const PipelineInfo &info) -> void
		{
			inputAssemblyCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssemblyCI.pNext = NULL;
			inputAssemblyCI.primitiveRestartEnable = VK_FALSE;
			inputAssemblyCI.topology = VkConverter::drawTypeToTopology(info.drawType);

			rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rs.polygonMode = VkConverter::polygonModeToVk(info.polygonMode);
			rs.cullMode = VkConverter::cullModeToVk(info.cullMode);

			// TODO there is a bug here.
			// because vulkan Y axis is up to down and different with opengl
			// http://anki3d.org/vulkan-coordinate-system/
			rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
			    //VK_FRONT_FACE_COUNTER_CLOCKWISE; ////info.flipY && info.swapChainTarget ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
			rs.depthClampEnable = VK_FALSE;
			rs.rasterizerDiscardEnable = VK_FALSE;
			rs.depthBiasEnable = (info.depthBiasEnabled ? VK_TRUE : VK_FALSE);
			rs.depthBiasConstantFactor = 0;
			rs.depthBiasClamp = 0;
			rs.depthBiasSlopeFactor = 0;
			rs.lineWidth = 1.0f;
			rs.pNext = NULL;
		}

		inline auto createColorBlend(VkPipelineColorBlendStateCreateInfo &cb, std::vector<VkPipelineColorBlendAttachmentState> &blendAttachState,
		                             const PipelineInfo &info, std::shared_ptr<VulkanRenderPass> renderPass) -> void
		{
			blendAttachState.resize(renderPass->getColorAttachmentCount());
			cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			cb.pNext = NULL;
			cb.flags = 0;
			for(unsigned int i = 0; i < blendAttachState.size(); i++) {
				blendAttachState[i] = VkPipelineColorBlendAttachmentState();
				blendAttachState[i].colorWriteMask = 0x0f;
				blendAttachState[i].alphaBlendOp = VK_BLEND_OP_ADD;
				blendAttachState[i].colorBlendOp = VK_BLEND_OP_ADD;
				blendAttachState[i].blendEnable = info.transparencyEnabled;
				if (info.transparencyEnabled) 
				{
					blendAttachState[i].srcAlphaBlendFactor = convertBlend(info.blendMode);
					blendAttachState[i].dstAlphaBlendFactor = convertBlend(info.dstBlendMode);
					blendAttachState[i].srcColorBlendFactor = convertBlend(info.blendMode);
					blendAttachState[i].dstColorBlendFactor = convertBlend(info.dstBlendMode);
				}
			}

			cb.attachmentCount = static_cast<uint32_t>(blendAttachState.size());
			cb.pAttachments = blendAttachState.data();
			cb.logicOpEnable = VK_FALSE;
			cb.logicOp = VK_LOGIC_OP_NO_OP;
			cb.blendConstants[0] = 1.0f;
			cb.blendConstants[1] = 1.0f;
			cb.blendConstants[2] = 1.0f;
			cb.blendConstants[3] = 1.0f;
		}

		inline auto createViewport(VkPipelineViewportStateCreateInfo &vp, std::vector<VkDynamicState> &dynamicState) -> void
		{
			vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			vp.pNext = NULL;
			vp.viewportCount = 1;
			vp.scissorCount = 1;
			vp.pScissors = NULL;
			vp.pViewports = NULL;
			dynamicState.emplace_back(VK_DYNAMIC_STATE_VIEWPORT);
			dynamicState.emplace_back(VK_DYNAMIC_STATE_SCISSOR);
		}

	} // namespace

	VulkanPipeline::VulkanPipeline(const PipelineInfo &info) { init(info); }

	VulkanPipeline::~VulkanPipeline()
	{
		PROFILE_FUNCTION();
		auto &deletionQueue = VulkanContext::getDeletionQueue();
		auto pipeline = this->pipeline;
		deletionQueue.emplace([pipeline] { vkDestroyPipeline(*VulkanDevice::get(), pipeline, VK_NULL_HANDLE); });
	}

	auto VulkanPipeline::init(const PipelineInfo &info) -> bool
	{
		PROFILE_FUNCTION();
		shader = info.shader;
		auto vkShader = std::static_pointer_cast<VulkanShader>(info.shader);
		computePipline = vkShader->isComputeShader();
		description = info;
		pipelineLayout = vkShader->getPipelineLayout();

		transitionAttachments();
		createFrameBuffers();
		// Pipeline
		std::vector<VkDynamicState> dynamicStateDescriptors;
		VkPipelineDynamicStateCreateInfo dynamicStateCI{};
		dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCI.pNext = NULL;
		dynamicStateCI.pDynamicStates = dynamicStateDescriptors.data();

		// Vertex layout
		VkVertexInputBindingDescription vertexBindingDescription{};
		VkPipelineVertexInputStateCreateInfo vi{};
		createVertexLayout(vertexBindingDescription, vi, vkShader);

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI{};
		VkPipelineRasterizationStateCreateInfo rs{};
		createRasterization(inputAssemblyCI, rs, info);

		VkPipelineColorBlendStateCreateInfo cb{};
		std::vector<VkPipelineColorBlendAttachmentState> blendAttachState;
		createColorBlend(cb, blendAttachState, info, std::static_pointer_cast<VulkanRenderPass>(renderPass));

		VkPipelineViewportStateCreateInfo vp{};
		createViewport(vp, dynamicStateDescriptors);

		if(info.depthBiasEnabled) {
			dynamicStateDescriptors.emplace_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
			depthBiasConstant = 1.25f;
			depthBiasSlope = 1.75f;
			depthBiasEnabled = true;
		} else {
			depthBiasConstant = 0.f;
			depthBiasSlope = 0.f;
			depthBiasEnabled = false;
		}

		VkPipelineDepthStencilStateCreateInfo ds{};
		createDepthStencil(ds, info);
		VkPipelineMultisampleStateCreateInfo ms{};
		createMultisample(ms);

		dynamicStateCI.dynamicStateCount = uint32_t(dynamicStateDescriptors.size());
		dynamicStateCI.pDynamicStates = dynamicStateDescriptors.data();

		VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
		graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphicsPipelineCreateInfo.pNext = NULL;
		graphicsPipelineCreateInfo.layout = pipelineLayout;
		graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		graphicsPipelineCreateInfo.basePipelineIndex = -1;
		graphicsPipelineCreateInfo.pVertexInputState = &vi;
		graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCI;
		graphicsPipelineCreateInfo.pRasterizationState = &rs;
		graphicsPipelineCreateInfo.pColorBlendState = &cb;
		graphicsPipelineCreateInfo.pTessellationState = VK_NULL_HANDLE;
		graphicsPipelineCreateInfo.pMultisampleState = &ms;
		graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCI;
		graphicsPipelineCreateInfo.pViewportState = &vp;
		graphicsPipelineCreateInfo.pDepthStencilState = &ds;
		graphicsPipelineCreateInfo.pStages = vkShader->getShaderStages().data();
		graphicsPipelineCreateInfo.stageCount = vkShader->getShaderStages().size();
		graphicsPipelineCreateInfo.renderPass = *std::static_pointer_cast<VulkanRenderPass>(renderPass);
		graphicsPipelineCreateInfo.subpass = 0;

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(*VulkanDevice::get(), VulkanDevice::get()->getPipelineCache(), 1, &graphicsPipelineCreateInfo,
		                                          VK_NULL_HANDLE, &pipeline));

		VulkanHelper::setObjectName(info.pipelineName, (uint64_t)pipeline, VK_OBJECT_TYPE_PIPELINE);

		return true;
	}

	auto VulkanPipeline::getWidth() -> uint32_t
	{
		if(description.swapChainTarget) { return GraphicsContext::get()->getSwapChain()->getCurrentImage()->getWidth(); }
		if(description.colorTargets[0]) return description.colorTargets[0]->getWidth();

		if(description.depthTarget) return description.depthTarget->getWidth();

		if(description.depthArrayTarget) return description.depthArrayTarget->getWidth();

		LOGW("Invalid pipeline width");

		return 0;
	}

	auto VulkanPipeline::getHeight() -> uint32_t
	{
		if(description.swapChainTarget) { return GraphicsContext::get()->getSwapChain()->getCurrentImage()->getHeight(); }
		if(description.colorTargets[0]) return description.colorTargets[0]->getHeight();

		if(description.depthTarget) return description.depthTarget->getHeight();

		if(description.depthArrayTarget) return description.depthArrayTarget->getHeight();

		LOGW("Invalid pipeline height");

		return 0;
	}

	auto VulkanPipeline::beginSecondary(const CommandBuffer *commandBuffer, uint32_t layer /*= 0*/, int32_t cubeFace /*= -1*/, int32_t mipMapLevel /*= 0*/)
	    -> void
	{
		PROFILE_FUNCTION();
		auto mipScale = std::pow(0.5, mipMapLevel);
		FrameBuffer *framebuffer = nullptr;
		transitionAttachments();
		if(description.swapChainTarget) {
			framebuffer = framebuffers[VulkanContext::get()->getSwapChain()->getCurrentImageIndex()].get();
		} else if(description.depthArrayTarget) {
			framebuffer = framebuffers[layer].get();
		} else {
			framebuffer = framebuffers[0].get();
		}
		renderPass->beginRenderPass(commandBuffer, (float *)&description.clearColor, framebuffer, SubPassContents::Secondary, getWidth() * mipScale,
		                            getHeight() * mipScale, cubeFace, mipMapLevel);
	}

	auto VulkanPipeline::bind(const CommandBuffer *cmdBuffer, uint32_t layer, int32_t cubeFace, int32_t mipMapLevel) -> FrameBuffer *
	{
		PROFILE_FUNCTION();
		FrameBuffer *framebuffer = nullptr;
		transitionAttachments();

		if(depthBiasEnabled)
			vkCmdSetDepthBias(static_cast<const VulkanCommandBuffer *>(cmdBuffer)->getCommandBuffer(), depthBiasConstant, 0.0f, depthBiasSlope);

		if(description.swapChainTarget) {
			framebuffer = framebuffers[VulkanContext::get()->getSwapChain()->getCurrentImageIndex()].get();
		} else if(description.depthArrayTarget) {
			framebuffer = framebuffers[layer].get();
		} else {
			framebuffer = framebuffers[0].get();
		}

		auto mipScale = std::pow(0.5, mipMapLevel);
		if(!cmdBuffer->isSecondary()) {
			renderPass->beginRenderPass(cmdBuffer, (float *)&description.clearColor, framebuffer, SubPassContents::Inline, getWidth() * mipScale,
			                            getHeight() * mipScale, cubeFace, mipMapLevel);
		} else {
			cmdBuffer->updateViewport(getWidth() * mipScale, getHeight() * mipScale);
		}

		vkCmdBindPipeline(static_cast<const VulkanCommandBuffer *>(cmdBuffer)->getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		return framebuffer;
	}

	auto VulkanPipeline::bind(const CommandBuffer *cmdBuffer, const ivec4 &viewport) -> FrameBuffer *
	{
		PROFILE_FUNCTION();
		FrameBuffer *framebuffer = nullptr;
		transitionAttachments();

		vkCmdSetDepthBias(static_cast<const VulkanCommandBuffer *>(cmdBuffer)->getCommandBuffer(), depthBiasConstant, 0.0f, depthBiasSlope);

		if(description.swapChainTarget) {
			framebuffer = framebuffers[VulkanContext::get()->getSwapChain()->getCurrentImageIndex()].get();
		} else {
			framebuffer = framebuffers[0].get();
		}

		if (!cmdBuffer->isSecondary())
			renderPass->beginRenderPass(cmdBuffer, (float *)&description.clearColor, framebuffer, SubPassContents::Inline, getWidth(), getHeight(), (int32_t *)&viewport);

		if (viewport.x >= 0)
			cmdBuffer->updateViewport(viewport.x, viewport.y, viewport.z, viewport.w);
		else
			cmdBuffer->updateViewport(getWidth(), getHeight());

		vkCmdBindPipeline(static_cast<const VulkanCommandBuffer *>(cmdBuffer)->getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		return framebuffer;
	}

	auto VulkanPipeline::end(const CommandBuffer *commandBuffer) -> void
	{
		PROFILE_FUNCTION();
		if(!commandBuffer->isSecondary()) renderPass->endRenderPass(commandBuffer);
	}

	auto VulkanPipeline::clearRenderTargets(const CommandBuffer *commandBuffer) -> void
	{
		PROFILE_FUNCTION();
		if(description.swapChainTarget) {
			for(uint32_t i = 0; i < GraphicsContext::get()->getSwapChain()->getSwapChainBufferCount(); i++) {
				RenderDevice::get()->clearRenderTarget(GraphicsContext::get()->getSwapChain()->getImage(i), commandBuffer);
			}
		}

		if(description.depthArrayTarget) { RenderDevice::get()->clearRenderTarget(description.depthArrayTarget, commandBuffer); }

		if(description.depthTarget) { RenderDevice::get()->clearRenderTarget(description.depthTarget, commandBuffer); }

		for(auto texture : description.colorTargets) {
			if(texture != nullptr) { RenderDevice::get()->clearRenderTarget(texture, commandBuffer); }
		}
	}

	auto VulkanPipeline::getRenderPass() -> std::shared_ptr<RenderPass> { return renderPass; }

	auto VulkanPipeline::getFrameBuffer() -> std::shared_ptr<FrameBuffer>
	{
		PROFILE_FUNCTION();
		if(description.swapChainTarget) {
			return framebuffers[VulkanContext::get()->getSwapChain()->getCurrentImageIndex()];
		} else {
			return framebuffers[0];
		}
	}

	auto VulkanPipeline::dispatchIndirect(const CommandBuffer *cmdBuffer, const StorageBuffer *ssbo, uint64_t offset) -> void
	{
		PROFILE_FUNCTION();
		auto vkCmd = static_cast<const VulkanCommandBuffer *>(cmdBuffer);
		auto vkBuffer = static_cast<const VulkanStorageBuffer *>(ssbo);
		vkCmdDispatchIndirect(vkCmd->getCommandBuffer(), vkBuffer->getHandle(), offset);
	}

	auto VulkanPipeline::drawIndexedIndirect(const CommandBuffer *cmdBuffer, const StorageBuffer *ssbo, uint32_t drawCount, uint32_t stride,
	                                         uint64_t offset /*= 0*/) -> void
	{
		PROFILE_FUNCTION();
		auto vkCmd = static_cast<const VulkanCommandBuffer *>(cmdBuffer);
		auto vkBuffer = static_cast<const VulkanStorageBuffer *>(ssbo);
		vkCmdDrawIndexedIndirect(vkCmd->getCommandBuffer(), vkBuffer->getHandle(), offset, drawCount, stride);
	}

	auto VulkanPipeline::drawIndexed(const CommandBuffer *cmdBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
	                                 uint32_t firstInstance) -> void
	{
		PROFILE_FUNCTION();
		auto vkCmd = static_cast<const VulkanCommandBuffer *>(cmdBuffer);
		vkCmdDrawIndexed(vkCmd->getCommandBuffer(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	auto VulkanPipeline::bufferBarrier(const CommandBuffer *commandBuffer, const std::vector<std::shared_ptr<StorageBuffer>> &buffers, bool read) -> void
	{
		PROFILE_FUNCTION();
		auto vkCmd = static_cast<const VulkanCommandBuffer *>(commandBuffer);
		std::vector<VkBufferMemoryBarrier> bufferBarriers;
		for(auto &ssbo : buffers) {
			auto vkBuffer = static_cast<VulkanStorageBuffer *>(ssbo.get());
			VkBufferMemoryBarrier memoryBarrier{};

			uint32_t srcAccessFlags = vkBuffer->getAccessFlagBits();
			uint32_t dstAccessFlags = read ? VK_ACCESS_SHADER_READ_BIT : VK_ACCESS_SHADER_WRITE_BIT;

			if(vkBuffer->isIndirect() && read) { dstAccessFlags = dstAccessFlags | VK_ACCESS_INDIRECT_COMMAND_READ_BIT; }

			memoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			memoryBarrier.srcAccessMask = srcAccessFlags;
			memoryBarrier.dstAccessMask = dstAccessFlags;
			memoryBarrier.buffer = vkBuffer->getHandle();
			memoryBarrier.offset = 0;
			memoryBarrier.size = VK_WHOLE_SIZE;

			vkBuffer->setAccessFlagBits(dstAccessFlags);

			bufferBarriers.emplace_back(memoryBarrier);
		}
		// TODO...................
		vkCmdPipelineBarrier(vkCmd->getCommandBuffer(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, 0,
		                     bufferBarriers.size(), bufferBarriers.data(), 0, 0);
	}

	auto VulkanPipeline::transitionAttachments() -> void
	{
		PROFILE_FUNCTION();

		auto commandBuffer = static_cast<VulkanCommandBuffer *>(GraphicsContext::get()->getSwapChain()->getCurrentCommandBuffer());
		/*if (!commandBuffer->isRecording())
		{
		        commandBuffer = nullptr;
		}*/

		if(description.swapChainTarget) {
			for(uint32_t i = 0; i < GraphicsContext::get()->getSwapChain()->getSwapChainBufferCount(); i++) {
				((VulkanTexture2D *)GraphicsContext::get()->getSwapChain()->getImage(i).get())
				    ->transitionImage(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, commandBuffer);
			}
		}

		if(description.depthArrayTarget) {
			((VulkanTextureDepthArray *)description.depthArrayTarget.get())
			    ->transitionImage(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, commandBuffer);
		}

		if(description.depthTarget) {
			((VulkanTextureDepth *)description.depthTarget.get())->transitionImage(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, commandBuffer);
		}

		for(auto texture : description.colorTargets) {
			if(texture != nullptr) {
				if(texture->getType() == TextureType::Color) {
					((VulkanTexture2D *)texture.get())->transitionImage(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, commandBuffer);
				} else if(texture->getType() == TextureType::Depth) {
					((VulkanTextureDepth *)texture.get())->transitionImage(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, commandBuffer);
				}
			}
		}
	}
	auto VulkanPipeline::createFrameBuffers() -> void
	{
		PROFILE_FUNCTION();
		std::vector<std::shared_ptr<Texture>> textures;

		if(description.swapChainTarget) {
			textures.emplace_back(GraphicsContext::get()->getSwapChain()->getCurrentImage());
		} else {
			for(auto texture : description.colorTargets) {
				if(texture) { textures.emplace_back(texture); }
			}
		}

		if(description.depthTarget) { textures.emplace_back(description.depthTarget); }

		if(description.depthArrayTarget) { textures.emplace_back(description.depthArrayTarget); }

		RenderPassInfo renderPassInfo;
		renderPassInfo.depthTarget = description.depthTarget;
		renderPassInfo.attachments = textures;
		renderPassInfo.clear = description.clearTargets;

		renderPass = RenderPass::create(renderPassInfo);

		FrameBufferInfo frameBufferInfo{};
		frameBufferInfo.width = getWidth();
		frameBufferInfo.height = getHeight();
		frameBufferInfo.renderPass = renderPass;
		frameBufferInfo.attachments = textures;

		if(description.swapChainTarget) {
			auto bufferCount = GraphicsContext::get()->getSwapChain()->getSwapChainBufferCount();

			for(uint32_t i = 0; i < bufferCount; i++) {
				// todo ...............
				frameBufferInfo.screenFBO = true;
				textures[0] = GraphicsContext::get()->getSwapChain()->getImage(i);
				frameBufferInfo.attachments = textures;
				framebuffers.emplace_back(FrameBuffer::create(frameBufferInfo));
			}
		} else if(description.depthArrayTarget) {
			auto depth = (VulkanTextureDepthArray *)description.depthArrayTarget.get();

			for(uint32_t i = 0; i < depth->getCount(); ++i) {
				frameBufferInfo.layer = i;
				frameBufferInfo.screenFBO = false;
				textures[0] = description.depthArrayTarget;
				frameBufferInfo.attachments = textures;

				framebuffers.emplace_back(FrameBuffer::create(frameBufferInfo));
			}
		} else {
			frameBufferInfo.attachments = textures;
			frameBufferInfo.screenFBO = false;
			framebuffers.emplace_back(FrameBuffer::create(frameBufferInfo));
		}
	}
}; // namespace maple
