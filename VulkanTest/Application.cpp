#include "Application.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "Input.h"
#include <stdexcept>
#include <array>
#include <numeric>
#include <chrono>
#include "VESimpleRenderSystem.h"
#include "VEBuffer.h"

namespace VE
{

    struct GlobalUbo 
    {
        alignas(16) glm::mat4 projectionView{ 1.f };
        alignas(16) glm::vec3 lightDirection = glm::normalize(glm::vec3(1.f, -3.f, -1.f));
    };
    
    constexpr float MAX_FRAME_TIME = 0.1f;

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
        std::vector<std::unique_ptr<VEBuffer>> uboBuffers(VESwapChain::MAX_FRAMES_IN_FLIGHT);

        for (int i = 0; i < uboBuffers.size(); ++i) 
        {
            uboBuffers[i] = std::make_unique<VEBuffer>(
                m_device,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                );

            uboBuffers[i]->map();
        }

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
                int frameIndex = m_renderer.GetFrameIndex();
                FrameInfo frameInfo
                {
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    camera
                };

                //Update
                GlobalUbo ubo{};
                ubo.projectionView = camera.GetProjection() * camera.GetView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                //Render
                //This way we can add multiple render passes (Shadows, reflection, etc)
                m_renderer.BeginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.RenderGameObjects(frameInfo, m_gameObjects);
                m_renderer.EndSwapChainRenderPass(commandBuffer);
                m_renderer.EndFrame();
            }
        }
        vkDeviceWaitIdle(m_device.device()); //block CPU until all GPU operations complete.
    }

    void Application::LoadGameObjects()
    {
        std::shared_ptr<VEModel> veModel = VEModel::CreateModelFromFile(m_device, "models/colored_cube.obj");
        std::shared_ptr<VEModel> veModel_2 = VEModel::CreateModelFromFile(m_device, "models/cube.obj");
        std::shared_ptr<VEModel> veModel_3 = VEModel::CreateModelFromFile(m_device, "models/flat_vase.obj");
        std::shared_ptr<VEModel> veModel_4 = VEModel::CreateModelFromFile(m_device, "models/smooth_vase.obj");


        auto gameObject_1 = VEGameObject::CreateGameObject();
        gameObject_1.m_model = veModel;
        gameObject_1.m_transformComponent.translation = { 0.f, 0.f, 2.5f };
        gameObject_1.m_transformComponent.scale = { 0.5f, 0.5f, .5f };
        gameObject_1.m_transformComponent.rotation = { glm::radians(180.f), glm::radians(0.f), glm::radians(0.f) };

        auto gameObject_2 = VEGameObject::CreateGameObject();
        gameObject_2.m_model = veModel_2;
        gameObject_2.m_transformComponent.translation = { -2.3f, -0.4f, 3.5f };
        gameObject_2.m_transformComponent.scale = { 0.5f, 0.5f, .5f };
        gameObject_2.m_transformComponent.rotation = { glm::radians(45.f), glm::radians(-4.f), glm::radians(32.f) };

        auto gameObject_3 = VEGameObject::CreateGameObject();
        gameObject_3.m_model = veModel_3;
        gameObject_3.m_transformComponent.translation = { 1.1f, 2.f, 5.5f };
        //gameObject_3.m_transformComponent.scale = { 0.5f, 0.5f, .5f };
        gameObject_3.m_transformComponent.rotation = { glm::radians(32.f), glm::radians(-45.f), glm::radians(15.f) };

        auto gameObject_4 = VEGameObject::CreateGameObject();
        gameObject_4.m_model = veModel_4;
        gameObject_4.m_transformComponent.translation = { 2.1f, 2.f, 6.5f };
        gameObject_4.m_transformComponent.scale = { 2.5f, 1.5f, 2.5f };
        gameObject_4.m_transformComponent.rotation = { glm::radians(15.f), glm::radians(-3.f), glm::radians(45.f) };

        m_gameObjects.push_back(std::move(gameObject_1));
        m_gameObjects.push_back(std::move(gameObject_2));
        m_gameObjects.push_back(std::move(gameObject_3));
        m_gameObjects.push_back(std::move(gameObject_4));



    }
}
