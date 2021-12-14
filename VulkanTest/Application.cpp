#include "Application.h"
#include <stdexcept>
#include <array>
namespace VE 
{
    Application::Application()
    {
        LoadModels();
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
    
    void Application::LoadModels()
    {
        std::vector<VEModel::Vertex> vertices{
            {{0.0f, -0.5f}, {1.0f,0.0f,0.0f}},
            {{0.5f, 0.5f}, {0.0f,1.0f,0.0f}},
            {{-0.5f, 0.5f}, {0.0f,0.0f,1.0f}}
        };

        m_model = std::make_unique<VEModel>(m_device, vertices);
    }

    void Application::CreatePipelineLayout()
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create Pipeline Layout");
        }
    }

    void Application::CreatePipeline()
    {
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
        m_SwapChain = std::make_unique<VESwapChain>(m_device, extent);
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
        clearValues[0].color = { 0.1f, 0.1f, 0.1f, 0.1f };
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


        m_pipeline->Bind(m_commandBuffers[imageIndex]); //Bind Graphics pipeline
        m_model->Bind(m_commandBuffers[imageIndex]); //Bind model that contains vertex data
        m_model->Draw(m_commandBuffers[imageIndex]); //Draw

        vkCmdEndRenderPass(m_commandBuffers[imageIndex]);
        if (vkEndCommandBuffer(m_commandBuffers[imageIndex]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to end recording Command Buffer!");
        }
    }
}
