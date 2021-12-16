#include "VESimpleRenderSystem.h"
#include <glm/gtc/constants.hpp>
#include <stdexcept>
#include <array>
namespace VE
{
    struct SimplePushConstantData //Temporary
    {
        glm::mat4 transform{ 1.f };
        glm::mat4 normalMatrix{ 1.f };
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

    void VESimpleRenderSystem::RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<VEGameObject>& gameObjects, const VECamera& camera)
    {
        m_pipeline->Bind(commandBuffer); //Bind Pipeline
        auto projectionView = camera.GetProjection() * camera.GetView();

        for (auto& obj : gameObjects)
        {
            SimplePushConstantData push{};
            auto modelMatrix = obj.m_transformComponent.mat4();
            push.transform = projectionView * modelMatrix; //Might want to do in the vertex Shader
            push.normalMatrix = obj.m_transformComponent.normalMatrix();

            vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
            obj.m_model->Bind(commandBuffer); //Bind model that contains vertex data

            obj.m_model->Draw(commandBuffer); //Draw
        }
    }
}
