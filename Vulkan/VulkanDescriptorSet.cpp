//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////

#include "VulkanDescriptorSet.h"
#include "../Console.h"
#include "Raytracing/VulkanAccelerationStructure.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanContext.h"
#include "VulkanDescriptorPool.h"
#include "VulkanDevice.h"
#include "VulkanHelper.h"
#include "VulkanIndexBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanRenderDevice.h"
#include "VulkanShader.h"
#include "VulkanStorageBuffer.h"
#include "VulkanTexture.h"
#include "VulkanUniformBuffer.h"
#include "VulkanVertexBuffer.h"


namespace maple
{
	namespace
	{
		inline auto transitionImageLayout(const CommandBuffer* cmd, Texture* texture, bool sampler2d, int32_t mipLevel)
		{
			if (!texture)
				return;

			const VulkanCommandBuffer* commandBuffer = (VulkanCommandBuffer*)cmd;
			if (texture->getType() == TextureType::Color)
			{
				if (sampler2d)
				{
					if (mipLevel >= 0)
					{
						if (((VulkanTexture2D*)texture)->getImageLayout(mipLevel) != VK_IMAGE_LAYOUT_GENERAL)
						{
							((VulkanTexture2D*)texture)->transitionImage2(VK_IMAGE_LAYOUT_GENERAL, commandBuffer, mipLevel);
						}
					}
					else
					{
						if (((VulkanTexture2D*)texture)->getImageLayout() != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
						{
							((VulkanTexture2D*)texture)->transitionImage(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, commandBuffer);
						}
					}
				}
				else
				{
					((VulkanTexture2D*)texture)->transitionImage(VK_IMAGE_LAYOUT_GENERAL, commandBuffer);
				}
			}
			else if (texture->getType() == TextureType::Color3D)
			{
				((VulkanTexture3D*)texture)->transitionImage2(VK_IMAGE_LAYOUT_GENERAL, commandBuffer, mipLevel);
			}
			else if (texture->getType() == TextureType::Cube)
			{
				if (((VulkanTextureCube*)texture)->getImageLayout(mipLevel) != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
				{
					((VulkanTextureCube*)texture)->transitionImage(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, commandBuffer);
				}
			}
			else if (texture->getType() == TextureType::Depth)
			{
				((VulkanTextureDepth*)texture)->transitionImage(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, commandBuffer);
			}
			else if (texture->getType() == TextureType::DepthArray)
			{
				((VulkanTextureDepthArray*)texture)->transitionImage(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, commandBuffer);
			}
		}
	}        // namespace

	VulkanDescriptorSet::VulkanDescriptorSet(const DescriptorInfo& info)
	{
		PROFILE_FUNCTION();
		framesInFlight = uint32_t(GraphicsContext::get()->getSwapChain()->getSwapChainBufferCount());

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo;
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

		if (info.pool == nullptr)
		{
			auto pool = std::static_pointer_cast<VulkanRenderDevice>(RenderDevice::get())->getDescriptorPool();
			descriptorSetAllocateInfo.descriptorPool = std::static_pointer_cast<VulkanDescriptorPool>(pool)->getHandle();
		}
		else
		{
			descriptorSetAllocateInfo.descriptorPool = static_cast<const VulkanDescriptorPool*>(info.pool)->getHandle();
		}

		descriptorPool = descriptorSetAllocateInfo.descriptorPool;

		descriptorSetAllocateInfo.pSetLayouts = static_cast<VulkanShader*>(info.shader)->getDescriptorLayout(info.layoutIndex);
		descriptorSetAllocateInfo.descriptorSetCount = info.count;
		descriptorSetAllocateInfo.pNext = nullptr;

		VkDescriptorSetVariableDescriptorCountAllocateInfo variableInfo{};

		if (info.variableCount > 0)
		{
			variableInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
			variableInfo.descriptorSetCount = 1;
			variableInfo.pDescriptorCounts = &info.variableCount;
			descriptorSetAllocateInfo.pNext = &variableInfo;
		}

		shader		= info.shader;
		descriptors = shader->getDescriptorInfo(info.layoutIndex);
		uniformBuffers.resize(framesInFlight);

		for (auto& descriptor : descriptors)
		{
			if (descriptor.type == DescriptorType::UniformBufferDynamic) 
			{
				dynamic = true;

				for (uint32_t frame = 0; frame < framesInFlight; frame++)
				{
					uniformBuffers[frame][descriptor.name] = nullptr;
				}

				UniformBufferInfo info{};
				info.members = descriptor.members;
				uniformBuffersData[descriptor.name] = info;
			}

			if (descriptor.type == DescriptorType::UniformBuffer)
			{
				for (uint32_t frame = 0; frame < framesInFlight; frame++)
				{
					auto buffer = UniformBuffer::create();
					buffer->init(descriptor.size, nullptr);
					uniformBuffers[frame][descriptor.name] = buffer;
				}

				Buffer localStorage;
				localStorage.allocate(descriptor.size);
				localStorage.initializeEmpty();

				UniformBufferInfo info{};
				info.localStorage = localStorage;
				info.hasUpdated[0] = false;
				info.hasUpdated[1] = false;
				info.hasUpdated[2] = false;

				info.members = descriptor.members;
				uniformBuffersData[descriptor.name] = info;
			}
			else if (descriptor.type == DescriptorType::Buffer)
			{
			}
		}

		descriptorSet.resize(framesInFlight, nullptr);
		for (uint32_t frame = 0; frame < framesInFlight; frame++)
		{
			descriptorDirty[frame] = true;
			VK_CHECK_RESULT(vkAllocateDescriptorSets(*VulkanDevice::get(), &descriptorSetAllocateInfo, &descriptorSet[frame]));
		}

		memset(imageInfoPool.data(), 0, sizeof(VkDescriptorImageInfo) * MAX_IMAGE_INFOS);
	}

	VulkanDescriptorSet::VulkanDescriptorSet(const LayoutBings& desc)
	{
		MAPLE_ASSERT(false, "To do implement.");
	}

	VulkanDescriptorSet::~VulkanDescriptorSet()
	{
		PROFILE_FUNCTION();
		for (uint32_t frame = 0; frame < framesInFlight; frame++)
		{
			auto  vkSet = descriptorSet[frame];
			auto  pool = descriptorPool;
			auto& deletionQueue = VulkanContext::getDeletionQueue();
			deletionQueue.emplace([vkSet, pool] { vkFreeDescriptorSets(*VulkanDevice::get(), pool, 1, &vkSet); });
		}
	}

	auto VulkanDescriptorSet::update(const CommandBuffer* commandBuffer) -> void
	{
		PROFILE_FUNCTION();

		int32_t descriptorWritesCount = 0;

		const auto vkCmd = static_cast<const VulkanCommandBuffer*>(commandBuffer);

		currentFrame = GraphicsContext::get()->getSwapChain()->getCurrentBufferIndex();

		for (auto& bufferInfo : uniformBuffersData)
		{
			if (bufferInfo.second.hasUpdated[currentFrame])
			{
				if (!bufferInfo.second.dynamic)
					uniformBuffers[currentFrame][bufferInfo.first]->setData(bufferInfo.second.localStorage.data);
				bufferInfo.second.hasUpdated[currentFrame] = false;
			}
		}

		for (auto& imageInfo : descriptors)
		{
			if (imageInfo.type == DescriptorType::ImageSampler || imageInfo.type == DescriptorType::Image)
			{
				if (!imageInfo.textures.empty())
				{
					for (uint32_t i = 0; i < imageInfo.textures.size(); i++)
					{
						if (imageInfo.textures[i])
						{
							transitionImageLayout(
								commandBuffer, imageInfo.textures[i].get(),
								imageInfo.type == DescriptorType::ImageSampler,
								imageInfo.mipmapLevel);
						}
					}
				}
			}
		}

		if (descriptorDirty[currentFrame])
		{
			descriptorDirty[currentFrame] = false;
			int32_t imageIndex = 0;
			int32_t index = 0;

			for (auto& imageInfo : descriptors)
			{
				if (imageInfo.type == DescriptorType::ImageSampler || imageInfo.type == DescriptorType::Image)
				{
					if (!imageInfo.textures.empty())
					{
						auto validCount = 0;
						for (uint32_t i = 0; i < imageInfo.textures.size(); i++)
						{
							if (imageInfo.textures[i])
							{
								const auto& des = *static_cast<VkDescriptorImageInfo*>(imageInfo.textures[i]->getDescriptorInfo(imageInfo.mipmapLevel, imageInfo.format));

								imageInfoPool[i + imageIndex] = des;
								validCount++;
							}
						}

						if (validCount > 0)
						{
							VkWriteDescriptorSet writeDescriptorSet{};
							writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
							writeDescriptorSet.dstSet = descriptorSet[currentFrame];
							writeDescriptorSet.descriptorType = VkConverter::descriptorTypeToVK(imageInfo.type);
							writeDescriptorSet.dstBinding = imageInfo.binding;
							writeDescriptorSet.pImageInfo = &imageInfoPool[imageIndex];
							writeDescriptorSet.descriptorCount = validCount;

							MAPLE_ASSERT(writeDescriptorSet.descriptorCount != 0, "writeDescriptorSet.descriptorCount should be greater than zero");

							writeDescriptorSetPool[descriptorWritesCount] = writeDescriptorSet;
							imageIndex += validCount;
							descriptorWritesCount++;
						}
					}
				}
				else if (imageInfo.type == DescriptorType::UniformBufferDynamic) 
				{
					auto buffer = std::static_pointer_cast<VulkanUniformBuffer>(uniformBuffers[currentFrame][imageInfo.name]);

					bufferInfoPool[index].buffer = buffer->getVkBuffer();
					bufferInfoPool[index].offset = imageInfo.offset;
					bufferInfoPool[index].range = imageInfo.size;//TODO...should be more dynamic later...

					VkWriteDescriptorSet writeDescriptorSet{};
					writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeDescriptorSet.dstSet = descriptorSet[currentFrame];
					writeDescriptorSet.descriptorType = VkConverter::descriptorTypeToVK(imageInfo.type);
					writeDescriptorSet.dstBinding = imageInfo.binding;
					writeDescriptorSet.pBufferInfo = &bufferInfoPool[index];
					writeDescriptorSet.descriptorCount = 1;

					writeDescriptorSetPool[descriptorWritesCount] = writeDescriptorSet;
					index++;
					descriptorWritesCount++;
				}
				else if (imageInfo.type == DescriptorType::UniformBuffer)
				{
					auto buffer = std::static_pointer_cast<VulkanUniformBuffer>(uniformBuffers[currentFrame][imageInfo.name]);

					bufferInfoPool[index].buffer = buffer->getVkBuffer();
					bufferInfoPool[index].offset = imageInfo.offset;
					bufferInfoPool[index].range =  imageInfo.size;

					VkWriteDescriptorSet writeDescriptorSet{};
					writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeDescriptorSet.dstSet = descriptorSet[currentFrame];
					writeDescriptorSet.descriptorType = VkConverter::descriptorTypeToVK(imageInfo.type);
					writeDescriptorSet.dstBinding = imageInfo.binding;
					writeDescriptorSet.pBufferInfo = &bufferInfoPool[index];
					writeDescriptorSet.descriptorCount = 1;

					writeDescriptorSetPool[descriptorWritesCount] = writeDescriptorSet;
					index++;
					descriptorWritesCount++;
				}
				else if (imageInfo.type == DescriptorType::Buffer)
				{
					auto& buffers = ssbos[imageInfo.name];

					int32_t i = 0;

					for (auto& ssbo : buffers)
					{
						bufferInfoPool[index + i].buffer = (VkBuffer)ssbo->handle();
						bufferInfoPool[index + i].offset = imageInfo.offset;
						bufferInfoPool[index + i].range = imageInfo.size;
						i++;
					}

					VkWriteDescriptorSet writeDescriptorSet{};
					writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeDescriptorSet.dstSet = descriptorSet[currentFrame];
					writeDescriptorSet.descriptorType = VkConverter::descriptorTypeToVK(imageInfo.type);
					writeDescriptorSet.dstBinding = imageInfo.binding;
					writeDescriptorSet.pBufferInfo = &bufferInfoPool[index];
					writeDescriptorSet.descriptorCount = i;

					MAPLE_ASSERT(i != 0, "descriptorCount should not be zero");
					writeDescriptorSetPool[descriptorWritesCount] = writeDescriptorSet;
					index += i;
					descriptorWritesCount++;
				}
				else if (imageInfo.type == DescriptorType::AccelerationStructure)
				{
					auto acc = std::static_pointer_cast<VulkanAccelerationStructure>(accelerationStructures[imageInfo.name]);

					VkWriteDescriptorSetAccelerationStructureKHR descriptorAs{};
					descriptorAs.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
					descriptorAs.pNext = nullptr;
					descriptorAs.accelerationStructureCount = 1;
					descriptorAs.pAccelerationStructures = &acc->getAccelerationStructure();

					VkWriteDescriptorSet writeDescriptorSet{};
					writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeDescriptorSet.dstSet = descriptorSet[currentFrame];
					writeDescriptorSet.descriptorType = VkConverter::descriptorTypeToVK(imageInfo.type);
					writeDescriptorSet.dstBinding = imageInfo.binding;
					writeDescriptorSet.descriptorCount = 1;
					writeDescriptorSet.pNext = &descriptorAs;

					writeDescriptorSetPool[descriptorWritesCount] = writeDescriptorSet;
					descriptorWritesCount++;
				}
			}

			if (descriptorWritesCount > 0)
				vkUpdateDescriptorSets(*VulkanDevice::get(), descriptorWritesCount, writeDescriptorSetPool.data(), 0, nullptr);
		}
	}

	auto VulkanDescriptorSet::initUpdate() ->void
	{
		int32_t imageIndex = 0;
		int32_t index = 0;
		uint32_t descriptorWritesCount = 0;

		for (auto currentFrame = 0; currentFrame <framesInFlight; currentFrame++)
		{
			for (auto& imageInfo : descriptors)
			{
				if (imageInfo.type == DescriptorType::ImageSampler || imageInfo.type == DescriptorType::Image)
				{
					if (!imageInfo.textures.empty())
					{
						auto validCount = 0;
						for (uint32_t i = 0; i < imageInfo.textures.size(); i++)
						{
							if (imageInfo.textures[i])
							{
								const auto& des = *static_cast<VkDescriptorImageInfo*>(imageInfo.textures[i]->getDescriptorInfo(imageInfo.mipmapLevel, imageInfo.format));

								imageInfoPool[i + imageIndex] = des;
								validCount++;
							}
						}

						if (validCount > 0)
						{
							VkWriteDescriptorSet writeDescriptorSet{};
							writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
							writeDescriptorSet.dstSet = descriptorSet[currentFrame];
							writeDescriptorSet.descriptorType = VkConverter::descriptorTypeToVK(imageInfo.type);
							writeDescriptorSet.dstBinding = imageInfo.binding;
							writeDescriptorSet.pImageInfo = &imageInfoPool[imageIndex];
							writeDescriptorSet.descriptorCount = validCount;

							MAPLE_ASSERT(writeDescriptorSet.descriptorCount != 0, "writeDescriptorSet.descriptorCount should be greater than zero");

							writeDescriptorSetPool[descriptorWritesCount] = writeDescriptorSet;
							imageIndex += validCount;
							descriptorWritesCount++;
						}
					}
				}
				else if (imageInfo.type == DescriptorType::UniformBuffer || imageInfo.type == DescriptorType::UniformBufferDynamic)
				{
					auto buffer = std::static_pointer_cast<VulkanUniformBuffer>(uniformBuffers[currentFrame][imageInfo.name]);

					bufferInfoPool[index].buffer = buffer->getVkBuffer();
					bufferInfoPool[index].offset = imageInfo.offset;
					bufferInfoPool[index].range = imageInfo.size;

					VkWriteDescriptorSet writeDescriptorSet{};
					writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeDescriptorSet.dstSet = descriptorSet[currentFrame];
					writeDescriptorSet.descriptorType = VkConverter::descriptorTypeToVK(imageInfo.type);
					writeDescriptorSet.dstBinding = imageInfo.binding;
					writeDescriptorSet.pBufferInfo = &bufferInfoPool[index];
					writeDescriptorSet.descriptorCount = 1;

					writeDescriptorSetPool[descriptorWritesCount] = writeDescriptorSet;
					index++;
					descriptorWritesCount++;
				}
				else if (imageInfo.type == DescriptorType::Buffer)
				{
					auto& buffers = ssbos[imageInfo.name];

					int32_t i = 0;

					for (auto& ssbo : buffers)
					{
						bufferInfoPool[index + i].buffer = (VkBuffer)ssbo->handle();
						bufferInfoPool[index + i].offset = imageInfo.offset;
						bufferInfoPool[index + i].range = imageInfo.size;
						i++;
					}

					VkWriteDescriptorSet writeDescriptorSet{};
					writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeDescriptorSet.dstSet = descriptorSet[currentFrame];
					writeDescriptorSet.descriptorType = VkConverter::descriptorTypeToVK(imageInfo.type);
					writeDescriptorSet.dstBinding = imageInfo.binding;
					writeDescriptorSet.pBufferInfo = &bufferInfoPool[index];
					writeDescriptorSet.descriptorCount = i;

					MAPLE_ASSERT(i != 0, "descriptorCount should not be zero");
					writeDescriptorSetPool[descriptorWritesCount] = writeDescriptorSet;
					index += i;
					descriptorWritesCount++;
				}
				else if (imageInfo.type == DescriptorType::AccelerationStructure)
				{
					auto acc = std::static_pointer_cast<VulkanAccelerationStructure>(accelerationStructures[imageInfo.name]);

					VkWriteDescriptorSetAccelerationStructureKHR descriptorAs{};
					descriptorAs.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
					descriptorAs.pNext = nullptr;
					descriptorAs.accelerationStructureCount = 1;
					descriptorAs.pAccelerationStructures = &acc->getAccelerationStructure();

					VkWriteDescriptorSet writeDescriptorSet{};
					writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeDescriptorSet.dstSet = descriptorSet[currentFrame];
					writeDescriptorSet.descriptorType = VkConverter::descriptorTypeToVK(imageInfo.type);
					writeDescriptorSet.dstBinding = imageInfo.binding;
					writeDescriptorSet.descriptorCount = 1;
					writeDescriptorSet.pNext = &descriptorAs;

					writeDescriptorSetPool[descriptorWritesCount] = writeDescriptorSet;
					descriptorWritesCount++;
				}
			}
		}

		if (descriptorWritesCount > 0)
			vkUpdateDescriptorSets(*VulkanDevice::get(), descriptorWritesCount, writeDescriptorSetPool.data(), 0, nullptr);
	}

	auto VulkanDescriptorSet::getDescriptorSet() -> VkDescriptorSet
	{
		return descriptorSet[currentFrame];
	}

	auto VulkanDescriptorSet::setTexture(const std::string& name, const std::vector<std::shared_ptr<Texture>>& textures, int32_t mipLevel, bool forceRefreshCache) -> void
	{
		bool set = false;
		for (auto& descriptor : descriptors)
		{
			if ((descriptor.type == DescriptorType::ImageSampler || descriptor.type == DescriptorType::Image) && descriptor.name == name)
			{
				bool textureUpdated = forceRefreshCache;

				for (auto tex : textures)
				{
					if (tex->isUpdated())
					{
						textureUpdated = true;
						tex->setUpdate(false);
					}
				}

				if (descriptor.mipmapLevel != mipLevel || textureUpdated || textures != descriptor.textures)
				{
					descriptor.textures = textures;
					descriptor.mipmapLevel = mipLevel;
					descriptorDirty[0] = true;
					descriptorDirty[1] = true;
					descriptorDirty[2] = true;
				}

				set = true;
			}
		}
		if (!set)
			LOGW("did not find texture {0} in Descriptor", name);
	}

	auto VulkanDescriptorSet::setTexture(const std::string& name, const std::shared_ptr<Texture>& texture, int32_t mipLevel, bool forceRefreshCache) -> void
	{
		PROFILE_FUNCTION();
		MAPLE_ASSERT(texture != nullptr, "Texture should not be null");
		setTexture(name, std::vector<std::shared_ptr<Texture>>{texture}, mipLevel, forceRefreshCache);
	}

	auto VulkanDescriptorSet::setBuffer(const std::string& name, const std::shared_ptr<UniformBuffer>& buffer) -> void
	{
		for (auto & uniform : uniformBuffers)
		{
			for (auto & pair : uniform)
			{
				if (pair.first == name) 
				{
					pair.second = buffer;
				}
			}
		}
	}

	auto VulkanDescriptorSet::getUnifromBuffer(const std::string& name) -> std::shared_ptr<UniformBuffer>
	{
		return nullptr;
	}

	auto VulkanDescriptorSet::setUniform(const std::string& bufferName, const std::string& uniformName, const void* data) -> void
	{
		PROFILE_FUNCTION();
		if (auto iter = uniformBuffersData.find(bufferName); iter != uniformBuffersData.end())
		{
			for (auto& member : iter->second.members)
			{
				if (member.name == uniformName)
				{
					iter->second.localStorage.write(data, member.size, member.offset);
					iter->second.hasUpdated[0] = true;
					iter->second.hasUpdated[1] = true;
					iter->second.hasUpdated[2] = true;
					return;
				}
			}
		}
		LOGW("Uniform not found {0}.{1}", bufferName, uniformName);
	}

	auto VulkanDescriptorSet::setUniform(const std::string& bufferName, const std::string& uniformName, const void* data, uint32_t size) -> void
	{
		PROFILE_FUNCTION();
		if (auto iter = uniformBuffersData.find(bufferName); iter != uniformBuffersData.end())
		{
			for (auto& member : iter->second.members)
			{
				if (member.name == uniformName)
				{
					iter->second.localStorage.write(data, size, member.offset);
					iter->second.hasUpdated[0] = true;
					iter->second.hasUpdated[1] = true;
					iter->second.hasUpdated[2] = true;
					return;
				}
			}
		}
		LOGW("Uniform not found {0}.{1}", bufferName, uniformName);
	}

	auto VulkanDescriptorSet::setUniform(const std::string& bufferName, const void* data) -> void
	{
		PROFILE_FUNCTION();
		if (auto iter = uniformBuffersData.find(bufferName); iter != uniformBuffersData.end())
		{
			iter->second.localStorage.write(data, iter->second.localStorage.getSize(), 0);
			iter->second.hasUpdated[0] = true;
			iter->second.hasUpdated[1] = true;
			iter->second.hasUpdated[2] = true;
			return;
		}
		LOGW("Uniform not found {0}.{1}", bufferName);
	}

	auto VulkanDescriptorSet::setStorageBuffer(const std::string& name, std::shared_ptr<StorageBuffer> buffer) -> void
	{
		//ssbos[name]   = {vkBuffer->getHandle()};
		ssbos[name] = { buffer };
		if (buffer->isDirty())
		{
			buffer->setDirty(true);
			descriptorDirty[0] = true;
			descriptorDirty[1] = true;
			descriptorDirty[2] = true;
		}
	}

	auto VulkanDescriptorSet::setStorageBuffer(const std::string& name, std::shared_ptr<VertexBuffer> buffer) -> void
	{
		auto vkBuffer = std::static_pointer_cast<VulkanVertexBuffer>(buffer);
		ssbos[name] = { buffer };
	}

	auto VulkanDescriptorSet::setStorageBuffer(const std::string& name, std::shared_ptr<IndexBuffer> buffer) -> void
	{
		auto vkBuffer = std::static_pointer_cast<VulkanIndexBuffer>(buffer);
		ssbos[name] = { buffer };
	}

	auto VulkanDescriptorSet::setStorageBuffer(const std::string& name, const std::vector<std::shared_ptr<StorageBuffer>>& buffers) -> void
	{
		ssbos[name].clear();
		for (auto& buffer : buffers)
		{
			ssbos[name].emplace_back(buffer);
			if (buffer->isDirty())
			{
				buffer->setDirty(true);
				descriptorDirty[0] = true;
				descriptorDirty[1] = true;
				descriptorDirty[2] = true;
			}
		}
	}

	auto VulkanDescriptorSet::setStorageBuffer(const std::string& name, const std::vector<std::shared_ptr<IndexBuffer>>& buffers) -> void
	{
		ssbos[name].clear();
		for (auto& buffer : buffers)
		{
			ssbos[name].emplace_back(buffer);
		}
	}

	auto VulkanDescriptorSet::setStorageBuffer(const std::string& name, const std::vector<std::shared_ptr<VertexBuffer>>& buffers) -> void
	{
		ssbos[name].clear();
		for (auto& buffer : buffers)
		{
			ssbos[name].emplace_back(buffer);
		}
	}

	auto VulkanDescriptorSet::setAccelerationStructure(const std::string& name, const std::shared_ptr<AccelerationStructure>& structure) -> void
	{
		if (accelerationStructures[name] != structure)
		{
			accelerationStructures[name] = structure;
			descriptorDirty[0] = true;
			descriptorDirty[1] = true;
			descriptorDirty[2] = true;
		}
	}

	auto VulkanDescriptorSet::setName(const std::string& name) -> void
	{
		int32_t i = 0;
		for (auto desc : descriptorSet)
		{
			VulkanHelper::setObjectName(name + ":" + std::to_string(i), (uint64_t)desc, VK_OBJECT_TYPE_DESCRIPTOR_SET);
		}
	}
};        // namespace maple
