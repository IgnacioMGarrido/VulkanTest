#include "VESimpleRenderSystem.h"
#include <glm/gtc/constants.hpp>
#include <stdexcept>
#include <array>
namespace VE
{
    struct SimplePushConstantData //Temporary
    {
        glm::mat4 modelMatrix{ 1.f };
        glm::mat4 normalMatrix{ 1.f };
    };

    VESimpleRenderSystem::VESimpleRenderSystem(VEDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
        :m_device(device)
    {
        CreatePipelineLayout(globalSetLayout);
        CreatePipeline(renderPass);
    }

    VESimpleRenderSystem::~VESimpleRenderSystem()
    {
        vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
    }

    void VESimpleRenderSystem::CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout)
    {
        VkPushConstantRange pushConstantRange;
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
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

    void VESimpleRenderSystem::RenderGameObjects(FrameInfo& frameInfo, std::vector<VEGameObject>& gameObjects)
    {
        m_pipeline->Bind(frameInfo.commandBuffer); //Bind Pipeline

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr
        );

        for (auto& obj : gameObjects)
        {
            SimplePushConstantData push{};
            push.modelMatrix = obj.m_transformComponent.mat4(); //Might want to do in the vertex Shader
            push.normalMatrix = obj.m_transformComponent.normalMatrix();

            vkCmdPushConstants(frameInfo.commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
            obj.m_model->Bind(frameInfo.commandBuffer); //Bind model that contains vertex data

            obj.m_model->Draw(frameInfo.commandBuffer); //Draw
        }
    }
}
