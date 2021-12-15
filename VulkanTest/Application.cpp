#include "Application.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include "VESimpleRenderSystem.h"

namespace VE 
{
    struct SimplePushConstantData //Temporary
    {
        glm::mat2 transform{1.f};
        glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };

    Application::Application()
    {
        LoadGameObjects();
    }
    
    Application::~Application()
    {
        vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
    }

    void Application::Run()
    {
        VESimpleRenderSystem simpleRenderSystem{ m_device, m_renderer.GetSwapChainRenderPass() };

        while (!m_levelWindow.ShouldClose()) 
        {
            m_levelWindow.RetreivePollEvents();
            if (auto commandBuffer = m_renderer.BeginFrame()) 
            {
                //This way we can add multiple render passes (Shadows, reflection, etc)
                m_renderer.BeginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.RenderGameObjects(commandBuffer, m_gameObjects);
                m_renderer.EndSwapChainRenderPass(commandBuffer);
                m_renderer.EndFrame();
            }

        }
        vkDeviceWaitIdle(m_device.device()); //block CPU until all GPU operations complete.
    }
    
    void Application::LoadGameObjects()
    {
        std::vector<VEModel::Vertex> vertices{
            {{0.0f, -0.5f}, {1.0f,0.0f,0.0f}},
            {{0.5f, 0.5f}, {0.0f,1.0f,0.0f}},
            {{-0.5f, 0.5f}, {0.0f,0.0f,1.0f}}
        };

        auto model = std::make_shared<VEModel>(m_device, vertices);

        auto triangle = VEGameObject::CreateGameObject();
        triangle.m_model = model;
        triangle.m_color = { .1f, .6f, .1f };
        triangle.m_transformComponent.m_translataion.x = .0f;
        triangle.m_transformComponent.m_scale = { 1.0f, 1.0f };
        triangle.m_transformComponent.m_rotation = .25f * glm::two_pi<float>();

        m_gameObjects.push_back(std::move(triangle));
    }
}
