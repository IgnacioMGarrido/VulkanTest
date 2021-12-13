#pragma once
#include "VEWindow.h"
#include "VEPipeline.h"
#include "VEdevice.h"
namespace VE 
{
    class Application
    {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        void Run();
    private:
        VEwindow m_levelWindow{WIDTH,HEIGHT};
        VEDevice m_device{ m_levelWindow };
        VEPipeline m_pipeline{m_device, "Shaders/basic_shader.vert.spv", "Shaders/basic_shader.frag.spv", VEPipeline::DefaultPipelineConfigInfo(WIDTH, HEIGHT) };
    };

}

