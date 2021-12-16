#include "Application.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "Input.h"
#include <stdexcept>
#include <array>
#include <chrono>
#include "VESimpleRenderSystem.h"

namespace VE
{
    constexpr float MAX_FRAME_TIME = 0.1f;
    std::unique_ptr<VEModel> createCubeModel(VEDevice& device, glm::vec3 offset)
    {
        VEModel::Builder modelBuilder{};

        modelBuilder.vertices =
        {
            {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
            
            // right face (yellow)
            {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
            
            // top face (orange, remember y axis points down)
            {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            
            // bottom face (red)
            {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            
            // nose face (blue)
            {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            
            // tail face (green)
            {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        };
        for (auto& v : modelBuilder.vertices) {
            v.position += offset;
        }

        modelBuilder.indices = { 0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
                          12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21 };

        return std::make_unique<VEModel>(device, modelBuilder);
    }

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
        VECamera camera{};
        //camera.SetViewDirection(glm::vec3{0.f}, glm::vec3(0.5f, 0.1f, 1.f));
        camera.SetViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

        auto cameraGameObject = VEGameObject::CreateGameObject();
        KeyboardMovementInput cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!m_levelWindow.ShouldClose())
        {
            m_levelWindow.RetreivePollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            frameTime = glm::min(frameTime, MAX_FRAME_TIME);

            cameraController.MoveInPlaneXZ(m_levelWindow.GetGLFWwindow(), frameTime, cameraGameObject);
            camera.SetViewYXZ(cameraGameObject.m_transformComponent.translation, cameraGameObject.m_transformComponent.rotation);

            float aspect = m_renderer.GetAspectRatio();
            //camera.SetOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
            camera.SetPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

            if (auto commandBuffer = m_renderer.BeginFrame())
            {
                //This way we can add multiple render passes (Shadows, reflection, etc)
                m_renderer.BeginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.RenderGameObjects(commandBuffer, m_gameObjects, camera);
                m_renderer.EndSwapChainRenderPass(commandBuffer);
                m_renderer.EndFrame();
            }
        }
        vkDeviceWaitIdle(m_device.device()); //block CPU until all GPU operations complete.
    }

    void Application::LoadGameObjects()
    {
        std::shared_ptr<VEModel> veModel = createCubeModel(m_device, { 0.f, 0.f, 0.f });

        auto cube = VEGameObject::CreateGameObject();
        cube.m_model = veModel;
        cube.m_transformComponent.translation = { 0.f, 0.f, 2.5f };
        cube.m_transformComponent.scale = { 0.5f, 0.5f, .5f };

        auto cube_2 = VEGameObject::CreateGameObject();
        cube_2.m_model = veModel;
        cube_2.m_transformComponent.translation = { -0.3f, -0.4f, 1.5f };
        cube_2.m_transformComponent.scale = { 0.5f, 0.5f, .5f };
        cube_2.m_transformComponent.rotation = { glm::radians(45.f), glm::radians(-4.f), glm::radians(32.f) };

        auto cube_3 = VEGameObject::CreateGameObject();
        cube_3.m_model = veModel;
        cube_3.m_transformComponent.translation = { 1.1f, 2.f, 5.5f };
        cube_3.m_transformComponent.scale = { 0.5f, 0.5f, .5f };
        cube_3.m_transformComponent.rotation = { glm::radians(32.f), glm::radians(-45.f), glm::radians(15.f) };

        m_gameObjects.push_back(std::move(cube));
        m_gameObjects.push_back(std::move(cube_2));
        m_gameObjects.push_back(std::move(cube_3));


    }
}
