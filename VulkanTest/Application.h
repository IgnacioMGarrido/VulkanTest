#pragma once
#include "VEWindow.h"
#include "VEdevice.h"
#include "VERenderer.h"
#include "VEModel.h"
#include "VEGameObject.h"
#include "VEDescriptors.h"
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

    private:
        VEwindow m_levelWindow{ WIDTH, HEIGHT };
        VEDevice m_device{ m_levelWindow };
        VERenderer m_renderer{ m_levelWindow, m_device };
        VkPipelineLayout m_pipelineLayout;
        std::unique_ptr<VEDescriptorPool> m_globalDescriptorPool{};
        std::vector<VEGameObject> m_gameObjects;
    };

}