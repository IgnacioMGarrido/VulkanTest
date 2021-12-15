#pragma once
#include "VEWindow.h"
#include "VEPipeline.h"
#include "VEdevice.h"
#include "VERenderer.h"
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
        void RenderGameObjects(VkCommandBuffer commandBuffer);
    private:
        VEwindow m_levelWindow{WIDTH,HEIGHT};
        VEDevice m_device{ m_levelWindow };
        VERenderer m_renderer{ m_levelWindow, m_device };
        std::unique_ptr<VEPipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;
        std::vector<VEGameObject> m_gameObjects;
    };

}