//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              		//
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../DescriptorSet.h"
#include "../Buffer.h"
#include "VulkanHelper.h"

namespace maple
{
	constexpr int32_t MAX_BUFFER_INFOS      = 2048;
	constexpr int32_t MAX_IMAGE_INFOS       = 2048;
	constexpr int32_t MAX_WRITE_DESCTIPTORS = 32;

	class StorageBuffer;
	class VulkanBuffer;
	class GPUBuffer;

	class VulkanDescriptorSet final : public DescriptorSet
	{
	  public:
		VulkanDescriptorSet(const DescriptorInfo &info);
		VulkanDescriptorSet(const LayoutBings &desc);
		~VulkanDescriptorSet();
		auto update(const CommandBuffer *commandBuffer) -> void override;
		auto initUpdate() ->void override;

		inline auto setDynamicOffset(uint32_t offset) -> void override
		{
			dynamicOffset = offset;
		}
		inline auto getDynamicOffset() const -> uint32_t override
		{
			return dynamicOffset;
		}

		inline auto isDynamic() const
		{
			return dynamic;
		}

		auto getDescriptorSet() -> VkDescriptorSet;

		auto setTexture(const std::string &name, const std::vector<std::shared_ptr<Texture>> &textures, int32_t mipLevel = -1, bool forceRefreshCache = false) -> void override;
		auto setTexture(const std::string &name, const std::shared_ptr<Texture> &textures, int32_t mipLevel = -1, bool forceRefreshCache = false) -> void override;
		auto setBuffer(const std::string &name, const std::shared_ptr<UniformBuffer> &buffer) -> void override;
		auto getUnifromBuffer(const std::string &name) -> std::shared_ptr<UniformBuffer> override;
		auto setUniform(const std::string &bufferName, const void *data) -> void override;
		auto setUniform(const std::string &bufferName, const std::string &uniformName, const void *data) -> void override;
		auto setUniform(const std::string &bufferName, const std::string &uniformName, const void *data, uint32_t size) -> void override;

		auto setStorageBuffer(const std::string &name, std::shared_ptr<StorageBuffer> buffer) -> void override;
		auto setStorageBuffer(const std::string &name, std::shared_ptr<VertexBuffer> buffer) -> void override;
		auto setStorageBuffer(const std::string &name, std::shared_ptr<IndexBuffer> buffer) -> void override;

		auto setStorageBuffer(const std::string &name, const std::vector<std::shared_ptr<StorageBuffer>> &buffer) -> void override;
		auto setStorageBuffer(const std::string &name, const std::vector<std::shared_ptr<VertexBuffer>> &buffer) -> void override;
		auto setStorageBuffer(const std::string &name, const std::vector<std::shared_ptr<IndexBuffer>> &buffer) -> void override;

		auto setStorageBuffer(const std::string& name, const std::vector<VertexBuffer*>& buffer) -> void override;
		auto setStorageBuffer(const std::string& name, const std::vector<IndexBuffer*>& buffer) -> void override;


		auto setAccelerationStructure(const std::string &name, const std::shared_ptr<AccelerationStructure> &structure) -> void override;

		auto setName(const std::string &name) -> void override;

		inline auto getDescriptors() const -> const std::vector<Descriptor> & override
		{
			return descriptors;
		}
		inline auto toIntID() const -> const uint64_t override
		{
			return (uint64_t) descriptorSet[currentFrame];
		};

	  private:
		uint32_t dynamicOffset      = 0;
		Shader * shader             = nullptr;
		bool     dynamic            = false;
		bool     descriptorDirty[3] = {};

		std::vector<Descriptor> descriptors;

		std::array<VkDescriptorBufferInfo, MAX_BUFFER_INFOS>    bufferInfoPool;
		std::array<VkDescriptorImageInfo, MAX_IMAGE_INFOS>      imageInfoPool;
		std::array<VkWriteDescriptorSet, MAX_WRITE_DESCTIPTORS> writeDescriptorSetPool;

		uint32_t framesInFlight = 0;

		struct UniformBufferInfo
		{
			std::vector<BufferMemberInfo> members;
			Buffer                        localStorage;
			bool                          dynamic        = false;
			bool                          hasUpdated[10] = {};
			uint32_t offset = 0;
		};

		std::vector<VkDescriptorSet>                                                 descriptorSet;
		std::vector<std::unordered_map<std::string, std::shared_ptr<UniformBuffer>>> uniformBuffers;
		std::unordered_map<std::string, std::vector<std::shared_ptr<GPUBuffer>>> ssbos;
		std::unordered_map<std::string, std::vector<GPUBuffer*>> ssbos2;
		std::unordered_map<std::string, std::shared_ptr<AccelerationStructure>>      accelerationStructures;
		std::unordered_map<std::string, UniformBufferInfo>                           uniformBuffersData;

		uint32_t currentFrame = 0;

		VkDescriptorPool descriptorPool = nullptr;
	};
};        // namespace maple
