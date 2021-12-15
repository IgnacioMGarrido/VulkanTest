#include "Application.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
namespace VE 
{
    struct SimplePushConstantData //Temporary
    {
        glm::mat2 transform{1.f};
        glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };

    Application::Application()
    {
        LoadGameObjects();
        CreatePipelineLayout();
        CreatePipeline();
    }
    
    Application::~Application()
    {
        vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
    }

    void Application::Run()
    {
        while (!m_levelWindow.ShouldClose()) 
        {
            m_levelWindow.RetreivePollEvents();
            if (auto commandBuffer = m_renderer.BeginFrame()) 
            {
                m_renderer.BeginSwapChainRenderPass(commandBuffer);
                RenderGameObjects(commandBuffer);
                m_renderer.EndSwapChainRenderPass(commandBuffer);
                m_renderer.EndFrame();
            }

        }
        vkDeviceWaitIdle(m_device.device()); //block CPU until all GPU operations complete.
    }
    
    void Application::LoadGameObjects()
    {
        std::vector<VEModel::Vertex> vertices{
            {{0.0f, -0.5f}, {1.0f,0.0f,0.0f}},
            {{0.5f, 0.5f}, {0.0f,1.0f,0.0f}},
            {{-0.5f, 0.5f}, {0.0f,0.0f,1.0f}}
        };

        auto model = std::make_shared<VEModel>(m_device, vertices);

        auto triangle = VEGameObject::CreateGameObject();
        triangle.m_model = model;
        triangle.m_color = { .1f, .6f, .1f };
        triangle.m_transformComponent.m_translataion.x = .0f;
        triangle.m_transformComponent.m_scale = { 1.0f, 1.0f };
        triangle.m_transformComponent.m_rotation = .25f * glm::two_pi<float>();

        m_gameObjects.push_back(std::move(triangle));
    }

    void Application::CreatePipelineLayout()
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

    void Application::CreatePipeline()
    {
        assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");
        PipelineConfigInfo pipelineConfig = {};
        VEPipeline::DefaultPipelineConfigInfo(pipelineConfig);

        pipelineConfig.renderPass = m_renderer.GetSwapChainRenderPass();
        pipelineConfig.pipelineLayout = m_pipelineLayout;
        m_pipeline = std::make_unique<VEPipeline>(m_device, "Shaders/basic_shader.vert.spv", "Shaders/basic_shader.frag.spv", pipelineConfig);
    }

    void Application::RenderGameObjects(VkCommandBuffer commandBuffer)
    {
        m_pipeline->Bind(commandBuffer); //Bind Pipeline
        for (auto& obj : m_gameObjects) 
        {
            obj.m_transformComponent.m_rotation = glm::mod(obj.m_transformComponent.m_rotation + 0.0001f, glm::two_pi<float>());
            SimplePushConstantData push{};
            push.offset = obj.m_transformComponent.m_translataion;
            push.color = obj.m_color;
            push.transform = obj.m_transformComponent.mat2();

            vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
            obj.m_model->Bind(commandBuffer); //Bind model that contains vertex data

            obj.m_model->Draw(commandBuffer); //Draw
        }
    }
}
