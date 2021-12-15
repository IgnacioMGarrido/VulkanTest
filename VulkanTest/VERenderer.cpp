#include "VERenderer.h"

#include <stdexcept>
#include <array>
namespace VE
{
    VERenderer::VERenderer(VEwindow& window, VEDevice& device)
        :m_window{window}
        , m_device{device}
    {
        RecreateSwapChain();
        CreateCommandBuffers();
    }

    VERenderer::~VERenderer()
    {
        FreeCommandBuffers();
    }

    VkCommandBuffer VERenderer::BeginFrame()
    {
        assert(!m_isFrameStarted && "Can't call BeginFrame while in progress");
        auto result = m_SwapChain->acquireNextImage(&m_currentImageIndex); //index of the frame we should render to Next

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            RecreateSwapChain();
            return nullptr;
        }

        if (result != VK_SUCCESS || result == VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("Failed to acquire swap chain image!");
        }
        m_isFrameStarted = true;

        auto commandBuffer = GetCurrentCommandBuffer();
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to begin recording Command Buffer!");
        }

        return commandBuffer;
    }

    void VERenderer::EndFrame()
    {
        assert(m_isFrameStarted && "Can't call EndFrame while frame is not in progress");
        auto commandBuffer = GetCurrentCommandBuffer();

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to end recording Command Buffer!");
        }

        auto result = m_SwapChain->submitCommandBuffers(&commandBuffer, &m_currentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.WasWindowResized())
        {
            m_window.ResetWindowResizedFlag();
            RecreateSwapChain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to Submit command Buffers!");
        }

        m_isFrameStarted = false;
        m_currentframeIndex = (m_currentframeIndex + 1) % VESwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void VERenderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer)
    {
        assert(m_isFrameStarted && "Can't call BeginSwapChainRenderPass while frame is not in progress");
        assert(commandBuffer == GetCurrentCommandBuffer() && "Cannot Begin render pass on command buffer from a different frame");

        //Configure render Pass.
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_SwapChain->getRenderPass();
        renderPassInfo.framebuffer = m_SwapChain->getFrameBuffer(m_currentImageIndex);

        renderPassInfo.renderArea.offset = { 0,0 };
        renderPassInfo.renderArea.extent = m_SwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { 0.01f, 0.01f, 0.01f, 0.01f };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        //Biging RenderPAss
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_SwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(m_SwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{ {0,0}, m_SwapChain->getSwapChainExtent() };
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    }

    void VERenderer::EndSwapChainRenderPass(VkCommandBuffer commandBuffer)
    {
        assert(m_isFrameStarted && "Can't call EndSwapChainRenderPass while frame is not in progress");
        assert(commandBuffer == GetCurrentCommandBuffer() && "Cannot End render pass on command buffer from a different frame");
        vkCmdEndRenderPass(commandBuffer);
    }

    void VERenderer::RecreateSwapChain()
    {
        auto extent = m_window.GetExtent();
        while (extent.width == 0 || extent.height == 0)
        {
            extent = m_window.GetExtent();
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
            std::shared_ptr<VESwapChain> oldSwapChain = std::move(m_SwapChain);
            m_SwapChain = std::make_unique<VESwapChain>(m_device, extent, oldSwapChain);

            if (!oldSwapChain->compareSwapFormat(*m_SwapChain.get())) 
            {
                throw std::runtime_error("SwapChain image or depth format has changed!");
            }
        }
    }

    void VERenderer::CreateCommandBuffers()
    {
        m_commandBuffers.resize(VESwapChain::MAX_FRAMES_IN_FLIGHT);

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

    void VERenderer::FreeCommandBuffers()
    {
        vkFreeCommandBuffers(m_device.device(), m_device.getCommandPool(), static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
        m_commandBuffers.clear();
    }
}
