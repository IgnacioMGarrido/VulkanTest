#include "VEModel.h"
#include "VEUtils.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <unordered_map>

namespace std 
{
    template<>
    struct hash<VE::VEModel::Vertex> 
    {
        size_t operator()(VE::VEModel::Vertex const &vertex) const 
        {
            size_t seed = 0;
            VE::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
}

namespace VE 
{
    VEModel::VEModel(VEDevice& device, const VEModel::Builder& builder)
        :m_device(device)
    {
        CreateVertexBuffers(builder.vertices);
        CreateIndexBuffers(builder.indices);
    }

    VEModel::~VEModel()
    {
        vkDestroyBuffer(m_device.device(), m_vertexBuffer, nullptr);
        vkFreeMemory(m_device.device(), m_vertexBufferMemory, nullptr);

        if (m_hasIndexBuffer) 
        {
            vkDestroyBuffer(m_device.device(), m_indexBuffer, nullptr);
            vkFreeMemory(m_device.device(), m_indexBufferMemory, nullptr);
        }
    }

    void VEModel::Bind(VkCommandBuffer commandBuffer)
    {
        VkBuffer buffers[] = { m_vertexBuffer };
        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (m_hasIndexBuffer) 
        {
            vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        }
    }

    void VEModel::Draw(VkCommandBuffer commandBuffer)
    {
        if (m_hasIndexBuffer) 
        {
            vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0);
        }
        else 
        {
            vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
        }
    }

    std::unique_ptr<VEModel> VEModel::CreateModelFromFile(VEDevice& device, const std::string& filepath)
    {
        Builder builder{};

        builder.LoadModels(filepath);
        return std::make_unique<VEModel>(device, builder);
    }

    void VEModel::CreateVertexBuffers(const std::vector<Vertex>& vertices)
    {
        m_vertexCount = static_cast<uint32_t>(vertices.size());
        assert(m_vertexCount >= 3 && "Vertex count must be at least 3");

        //number of bytes required per vertex * vertex Count = Total number of bytes required by the vertex buffer to store the vertices
        VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;


        //Create a staging Buffer in the device to whcih we can copy the data from the CPU
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        m_device.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, //Accesible from the CPU (Host)
            stagingBuffer,
            stagingBufferMemory);

        void* data;
        if (vkMapMemory(m_device.device(), stagingBufferMemory, 0, bufferSize, 0, &data) != VK_SUCCESS)
        {
            throw std::runtime_error("Couldn't map vertex memory");
        }
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(m_device.device(), stagingBufferMemory);

        m_device.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, //Can be used as a destination of a transfer command
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, //Accesible ONLY by the device (GPU)
            m_vertexBuffer,
            m_vertexBufferMemory);

        //Copy data from the staging buffer to the VertexBuffer
        m_device.copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

        //Free memory of the staging buffer
        vkDestroyBuffer(m_device.device(), stagingBuffer, nullptr);
        vkFreeMemory(m_device.device(), stagingBufferMemory, nullptr);
    }

    void VEModel::CreateIndexBuffers(const std::vector<uint32_t>& indices)
    {
        m_indexCount = static_cast<uint32_t>(indices.size());
        m_hasIndexBuffer = m_indexCount > 0;

        if (!m_hasIndexBuffer) 
        {
            return;
        }
        //number of bytes required per index * index Count = Total number of bytes required by the index buffer to store the vertices
        VkDeviceSize bufferSize = sizeof(indices[0]) * m_indexCount;
        //Create a staging Buffer in the device to whcih we can copy the data from the CPU
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        m_device.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, //Accesible from the CPU (Host)
            stagingBuffer,
            stagingBufferMemory);

        void* data;
        if (vkMapMemory(m_device.device(), stagingBufferMemory, 0, bufferSize, 0, &data) != VK_SUCCESS)
        {
            throw std::runtime_error("Couldn't map index memory");
        }
        memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(m_device.device(), stagingBufferMemory);

        m_device.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, //Can be used as a destination of a transfer command
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, //Accesible ONLY by the device (GPU)
            m_indexBuffer,
            m_indexBufferMemory);

        //Copy data from the staging buffer to the IndexBuffer
        m_device.copyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

        //Free memory of the staging buffer
        vkDestroyBuffer(m_device.device(), stagingBuffer, nullptr);
        vkFreeMemory(m_device.device(), stagingBufferMemory, nullptr);
    }

    std::vector<VkVertexInputBindingDescription> VEModel::Vertex::GetBindingDescriptions()
    {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescriptions;
    }
    
    std::vector<VkVertexInputAttributeDescription> VEModel::Vertex::GetAttributesDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attributesDescriptions(2);
        attributesDescriptions[0].binding = 0;
        attributesDescriptions[0].location = 0;
        attributesDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributesDescriptions[0].offset = offsetof(Vertex, position);

        attributesDescriptions[1].binding = 0;
        attributesDescriptions[1].location = 1;
        attributesDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributesDescriptions[1].offset = offsetof(Vertex, color);

        return attributesDescriptions;
    }

    void VEModel::Builder::LoadModels(const std::string& filepath)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) 
        {
            throw std::runtime_error(warn + err);
        }

        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices) 
            {
                Vertex vertex{};
                if (index.vertex_index >= 0) 
                {
                    vertex.position =
                    {
                         attrib.vertices[3 * index.vertex_index + 0],
                         attrib.vertices[3 * index.vertex_index + 1],
                         attrib.vertices[3 * index.vertex_index + 2]
                    };

                    auto colorIndex = 3 * index.vertex_index + 2;

                    if (colorIndex < attrib.colors.size()) // we support colors 
                    {
                        vertex.color =
                        {
                            attrib.colors[colorIndex - 2],
                            attrib.colors[colorIndex - 1],
                            attrib.colors[colorIndex - 0]
                        };
                    }
                    else // set default color of the model 
                    {
                        //vertex.color = { 1.f, .20f, .87f }; // Bright Green
                        //vertex.color = { 1.0f, 0.4f, 1.0f }; //Ultra Pink
                        vertex.color = { 1.f, 1.f, 1.f };  // White
                    }
                }

                if (index.normal_index >= 0)
                {
                    vertex.normal =
                    {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2],
                    };
                }

                if (index.texcoord_index >= 0)
                {
                    vertex.uv =
                    {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1],
                    };
                }

                if (uniqueVertices.count(vertex) == 0) //it is not already in the uniqueVertices map
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size()); //add to the map
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }
}