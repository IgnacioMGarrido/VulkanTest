#pragma once
#include "VEWindow.h"
#include "VEPipeline.h"
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
        VEPipeline m_pipeline{ "Shaders/basic_shader.vert.spv", "Shaders/basic_shader.frag.spv" };
    };

}

