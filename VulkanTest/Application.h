#pragma once
#include "VEWindow.h"
#include "VEPipeline.h"
#include "VEdevice.h"
#include "VESwapChain.h"
#include "VEModel.h"
#include "VEGameObject.h"
#include <memory>
#include <vector>
namespace VE 
{
    class Application
    {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;
        Application();
        ~Application();
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;
        void Run();
    private:
        void LoadGameObjects();
        void CreatePipelineLayout();
        void CreatePipeline();
        void CreateCommandBuffers();
        void FreeCommandBuffers();
        void DrawFrame();

        void RecreateSwapChain();
        void RecordCommandBuffer(int imageIndex);
        void RenderGameObjects(VkCommandBuffer commandBuffer);
    private:
        VEwindow m_levelWindow{WIDTH,HEIGHT};
        VEDevice m_device{ m_levelWindow };
        std::unique_ptr<VESwapChain> m_SwapChain;
        std::unique_ptr<VEPipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;
        std::vector<VkCommandBuffer> m_commandBuffers;
        std::vector<VEGameObject> m_gameObjects;
    };

}

