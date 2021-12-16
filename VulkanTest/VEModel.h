#pragma once
#include "VEDevice.h"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_ONE
#include <glm/glm.hpp>
#include <memory>
#include <vector>
namespace VE {
    
    class VEModel
    {
    public:

        struct Vertex 
        {
            glm::vec3 position{};
            glm::vec3 color{};
            glm::vec3 normal{};
            glm::vec2 uv{};

            bool operator==(const Vertex& rhs) const
            {
                return position == rhs.position && color == rhs.color && normal == rhs.normal && uv == rhs.uv;
            };

            static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> GetAttributesDescriptions();

        };

        struct Builder 
        {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            void LoadModels(const std::string& filepath);
        };

        VEModel(VEDevice& device, const VEModel::Builder& builder);
        ~VEModel();

        VEModel(const VEModel&) = delete;
        VEModel& operator=(const VEModel&) = delete;

        void Bind(VkCommandBuffer commandBuffer);
        void Draw(VkCommandBuffer commandBuffer);

        static std::unique_ptr<VEModel> CreateModelFromFile(VEDevice& device, const std::string& filepath);

    private:
        void CreateVertexBuffers(const std::vector<Vertex>& vertices);
        void CreateIndexBuffers(const std::vector<uint32_t>& indices);

    private:
        VEDevice& m_device;

        VkBuffer m_vertexBuffer;
        VkDeviceMemory m_vertexBufferMemory;
        uint32_t m_vertexCount;

        VkBuffer m_indexBuffer;
        VkDeviceMemory m_indexBufferMemory;
        uint32_t m_indexCount;

        bool m_hasIndexBuffer = false;
    };
}

