#include "VESimpleRenderSystem.h"
#include <glm/gtc/constants.hpp>
#include <stdexcept>
#include <array>
namespace VE
{
    struct SimplePushConstantData //Temporary
    {
        glm::mat4 transform{ 1.f };
        alignas(16) glm::vec3 color;
    };

    VESimpleRenderSystem::VESimpleRenderSystem(VEDevice& device, VkRenderPass renderPass)
        :m_device(device)
    {
        CreatePipelineLayout();
        CreatePipeline(renderPass);
    }

    VESimpleRenderSystem::~VESimpleRenderSystem()
    {
        vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
    }

    void VESimpleRenderSystem::CreatePipelineLayout()
    {
        VkPushConstantRange pushConstantRange;
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create Pipeline Layout");
        }
    }

    void VESimpleRenderSystem::CreatePipeline(VkRenderPass renderPass)
    {
        assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");
        PipelineConfigInfo pipelineConfig = {};
        VEPipeline::DefaultPipelineConfigInfo(pipelineConfig);

        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_pipelineLayout;
        m_pipeline = std::make_unique<VEPipeline>(m_device, "Shaders/basic_shader.vert.spv", "Shaders/basic_shader.frag.spv", pipelineConfig);
    }

    void VESimpleRenderSystem::RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<VEGameObject>& gameObjects)
    {
        m_pipeline->Bind(commandBuffer); //Bind Pipeline
        for (auto& obj : gameObjects)
        {
            obj.m_transformComponent.rotation.y = glm::mod(obj.m_transformComponent.rotation.y + 0.0001f, glm::two_pi<float>());
            obj.m_transformComponent.rotation.z = glm::mod(obj.m_transformComponent.rotation.z + 0.00005f, glm::two_pi<float>());
            obj.m_transformComponent.rotation.x = glm::mod(obj.m_transformComponent.rotation.x + 0.0001f, glm::two_pi<float>());


            SimplePushConstantData push{};
            push.color = obj.m_color;
            push.transform = obj.m_transformComponent.mat4();

            vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
            obj.m_model->Bind(commandBuffer); //Bind model that contains vertex data

            obj.m_model->Draw(commandBuffer); //Draw
        }
    }
}
