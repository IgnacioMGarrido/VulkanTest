#pragma once

#include "VEDevice.h"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace VE {

    class VEDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(VEDevice& device) : device{ device } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<VEDescriptorSetLayout> build() const;

        private:
            VEDevice& device;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        VEDescriptorSetLayout(
            VEDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~VEDescriptorSetLayout();
        VEDescriptorSetLayout(const VEDescriptorSetLayout&) = delete;
        VEDescriptorSetLayout& operator=(const VEDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        VEDevice& device;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class VEDescriptorWriter;
    };

    class VEDescriptorPool {
    public:
        class Builder {
        public:
            Builder(VEDevice& device) : device{ device } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<VEDescriptorPool> build() const;

        private:
            VEDevice& device;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        VEDescriptorPool(
            VEDevice& device,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~VEDescriptorPool();
        VEDescriptorPool(const VEDescriptorPool&) = delete;
        VEDescriptorPool& operator=(const VEDescriptorPool&) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        VEDevice& device;
        VkDescriptorPool descriptorPool;

        friend class VEDescriptorWriter;
    };

    class VEDescriptorWriter {
    public:
        VEDescriptorWriter(VEDescriptorSetLayout& setLayout, VEDescriptorPool& pool);

        VEDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        VEDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        VEDescriptorSetLayout& setLayout;
        VEDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}