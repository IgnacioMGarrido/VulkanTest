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
        RecreateSwapChain();
        CreateCommandBuffers();
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
            DrawFrame();
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
        assert(m_SwapChain != nullptr && "Cannot create pipeline before swapChain!");
        assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");
        PipelineConfigInfo pipelineConfig = {};
        VEPipeline::DefaultPipelineConfigInfo(pipelineConfig);

        pipelineConfig.renderPass = m_SwapChain->getRenderPass();
        pipelineConfig.pipelineLayout = m_pipelineLayout;
        m_pipeline = std::make_unique<VEPipeline>(m_device, "Shaders/basic_shader.vert.spv", "Shaders/basic_shader.frag.spv", pipelineConfig);
    }
    
    void Application::RecreateSwapChain()
    {
        auto extent = m_levelWindow.GetExtent();
        while (extent.width == 0 || extent.height == 0) 
        {
            extent = m_levelWindow.GetExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(m_device.device());
        m_SwapChain.reset();

        if (m_SwapChain == nullptr) 
        {
            m_SwapChain = std::make_unique<VESwapChain>(m_device, extent);
        }
        else 
        {
            m_SwapChain = std::make_unique<VESwapChain>(m_device, extent, std::move(m_SwapChain));
            if (m_SwapChain->imageCount() != m_commandBuffers.size()) 
            {
                FreeCommandBuffers();
                CreateCommandBuffers();
            }
        }
        CreatePipeline();
    }

    void Application::CreateCommandBuffers()
    {
        m_commandBuffers.resize(m_SwapChain->imageCount());
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_device.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());
        
        if (vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate Command Buffers!");
        }
    }

    void Application::FreeCommandBuffers()
    {
        vkFreeCommandBuffers(m_device.device(), m_device.getCommandPool(),static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
        m_commandBuffers.clear();
    }
    
    void Application::DrawFrame()
    {
        uint32_t imageIndex;
        auto result = m_SwapChain->acquireNextImage(&imageIndex); //index of the frame we should render to Next

        if (result == VK_ERROR_OUT_OF_DATE_KHR) 
        {
            RecreateSwapChain();
            return;
        }

        if (result != VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) 
        {
            throw std::runtime_error("Failed to acquire swap chain image!");
        }

        RecordCommandBuffer(imageIndex);
        result = m_SwapChain->submitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_levelWindow.WasWindowResized())
        {
            m_levelWindow.ResetWindowResizedFlag();
            RecreateSwapChain();
            return;
        }
        if (result != VK_SUCCESS) 
        {
            throw std::runtime_error("Failed to Submit command Buffers!");
        }
    }

    void Application::RecordCommandBuffer(int imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(m_commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to begin recording Command Buffer!");
        }

        //Configure render Pass.
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_SwapChain->getRenderPass();
        renderPassInfo.framebuffer = m_SwapChain->getFrameBuffer(imageIndex);

        renderPassInfo.renderArea.offset = { 0,0 };
        renderPassInfo.renderArea.extent = m_SwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { 0.01f, 0.01f, 0.01f, 0.01f };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        //Biging RenderPAss
        vkCmdBeginRenderPass(m_commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_SwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(m_SwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{ {0,0}, m_SwapChain->getSwapChainExtent() };
        vkCmdSetViewport(m_commandBuffers[imageIndex], 0, 1, &viewport);
        vkCmdSetScissor(m_commandBuffers[imageIndex], 0, 1, &scissor);

        RenderGameObjects(m_commandBuffers[imageIndex]);


        vkCmdEndRenderPass(m_commandBuffers[imageIndex]);
        if (vkEndCommandBuffer(m_commandBuffers[imageIndex]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to end recording Command Buffer!");
        }
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
