#pragma once
#pragma once
#include "VEWindow.h"
#include "VEdevice.h"
#include "VESwapChain.h"
#include "VEModel.h"

#include <memory>
#include <vector>
#include <cassert>
namespace VE
{
    class VERenderer
    {
    public:
        VERenderer(VEwindow& window, VEDevice& device);
        ~VERenderer();

        VERenderer(const VERenderer&) = delete;
        VERenderer& operator=(const VERenderer&) = delete;

        VkCommandBuffer BeginFrame();
        void EndFrame();

        void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void EndSwapChainRenderPass(VkCommandBuffer commandBuffer);

        VkRenderPass GetSwapChainRenderPass() const { return m_SwapChain->getRenderPass(); }
        float GetAspectRatio() const { return m_SwapChain->extentAspectRatio(); }
        bool IsFrameInProgress() const { return m_isFrameStarted; }
        VkCommandBuffer GetCurrentCommandBuffer() const 
        { 
            assert(m_isFrameStarted && "Cabbot get commandBuffer when frame is in progress");
            return m_commandBuffers[m_currentframeIndex];
        }

        int GetFrameIndex() const 
        {
            assert(m_isFrameStarted && "Cabbot get commandBuffer when frame is in progress"); 
            return m_currentframeIndex;
        }

    private:
        void CreateCommandBuffers();
        void FreeCommandBuffers();
        void RecreateSwapChain();

    private:
        VEwindow& m_window;
        VEDevice& m_device;
        std::unique_ptr<VESwapChain> m_SwapChain;
        std::vector<VkCommandBuffer> m_commandBuffers;

        uint32_t m_currentImageIndex;
        bool m_isFrameStarted;
        int m_currentframeIndex;
    };

}
