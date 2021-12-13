#pragma once
#include "VEWindow.h"

namespace VE 
{
    class Application
    {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        void Run();
    private:
        VEwindow m_levelWindow{};
    };

}

