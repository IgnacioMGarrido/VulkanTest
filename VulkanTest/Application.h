#pragma once
#include "VEWindow.h"
#include "VEPipeline.h"
#include "VEdevice.h"
#include "VESwapChain.h"

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
        void CreatePipelineLayout();
        void CreatePipeline();
        void CreateCommandBuffers();
        void DrawFrame();
    private:
        VEwindow m_levelWindow{WIDTH,HEIGHT};
        VEDevice m_device{ m_levelWindow };
        VESwapChain m_SwapChain{ m_device, m_levelWindow.GetExtent() };
        //VEPipeline m_pipeline{m_device, "Shaders/basic_shader.vert.spv", "Shaders/basic_shader.frag.spv", VEPipeline::DefaultPipelineConfigInfo(WIDTH, HEIGHT) };
        std::unique_ptr<VEPipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;
        std::vector<VkCommandBuffer> m_commandBuffers;
    };

}

