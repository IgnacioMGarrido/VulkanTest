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
#include "VESimplePhysicsSystem.h"

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
        m_globalDescriptorPool = VEDescriptorPool::Builder(m_device)
            .setMaxSets(VESwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VESwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VESwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
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

        auto globalSetLayout = VEDescriptorSetLayout::Builder(m_device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(VESwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); ++i) 
        {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            VEDescriptorWriter(*globalSetLayout, *m_globalDescriptorPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        VESimpleRenderSystem simpleRenderSystem{ m_device, m_renderer.GetSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
        VESimplePhysicsSystem simplePhysicsSystem{};
        VECamera camera{};
        //camera.SetViewDirection(glm::vec3{0.f}, glm::vec3(0.5f, 0.1f, 1.f));
        auto cameraGameObject = VEGameObject::CreateGameObject();
        cameraGameObject.m_transformComponent.translation = glm::vec3(0.f, -3.f, -7.5f);//0.f, -3.f, -5.5f
        camera.SetViewTarget(cameraGameObject.m_transformComponent.translation, glm::vec3(0.f, 0.f, 2.5f));
        
        KeyboardMovementInput paddleController{};
        KeyboardMovementInput cameraController{};


        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!m_levelWindow.ShouldClose())
        {
            m_levelWindow.RetreivePollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            frameTime = glm::min(frameTime, MAX_FRAME_TIME);

            //cameraController.MoveInPlaneXZ(m_levelWindow.GetGLFWwindow(), frameTime, cameraGameObject);
            paddleController.MoveLeftRight(m_levelWindow.GetGLFWwindow(), frameTime, m_gameObjects[0]);
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
                    camera,
                    globalDescriptorSets[frameIndex]
                };

                //Update
                GlobalUbo ubo{};
                ubo.projectionView = camera.GetProjection() * camera.GetView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                simplePhysicsSystem.SimulateGameObjects(frameInfo, m_gameObjects);

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
        std::shared_ptr<VEModel> veModel_2 = VEModel::CreateModelFromFile(m_device, "models/suzanne.obj");
        std::shared_ptr<VEModel> veModel_3 = VEModel::CreateModelFromFile(m_device, "models/cube.obj");
        std::shared_ptr<VEModel> veModel_4 = VEModel::CreateModelFromFile(m_device, "models/smooth_vase.obj");


        auto gameObject_1 = VEGameObject::CreateGameObject();
        gameObject_1.m_model = veModel;
        gameObject_1.m_transformComponent.translation = { 0.f, 0.f, 2.5f };
        gameObject_1.m_transformComponent.scale = { 1.f, 0.2f, .1f };
        gameObject_1.m_transformComponent.rotation = { glm::radians(180.f), glm::radians(0.f), glm::radians(0.f) };
        
        gameObject_1.m_colliderComponent.SetupCollider(gameObject_1.m_transformComponent);


        auto gameObject_2 = VEGameObject::CreateGameObject();
        gameObject_2.m_model = veModel_2;
        gameObject_2.m_transformComponent.translation = { .5f, -2.f, 2.5f };
        gameObject_2.m_transformComponent.scale = { 0.2f, 0.2f, 0.2f };
        gameObject_2.m_transformComponent.rotation = { glm::radians(180.f), glm::radians(0.f), glm::radians(0.f) };
        gameObject_2.m_rigidBodyComponent.velocity = { 0.f, -3.f };

        gameObject_2.m_colliderComponent.SetupCollider(gameObject_2.m_transformComponent);

        m_gameObjects.push_back(std::move(gameObject_1));
        m_gameObjects.push_back(std::move(gameObject_2));


        //Walls

        auto wall_1 = VEGameObject::CreateGameObject();
        wall_1.m_model = veModel_3;
        wall_1.m_transformComponent.translation = { .5f, -7.5f, 2.5f };
        wall_1.m_transformComponent.scale = { 100.2f, 0.2f, 0.2f };
        wall_1.m_transformComponent.rotation = { glm::radians(180.f), glm::radians(0.f), glm::radians(0.f) };

        wall_1.m_colliderComponent.SetupCollider(wall_1.m_transformComponent);
        m_gameObjects.push_back(std::move(wall_1));

        auto wall_2 = VEGameObject::CreateGameObject();
        wall_2.m_model = veModel_3;
        wall_2.m_transformComponent.translation = { .5f, 3.f, 2.5f };
        wall_2.m_transformComponent.scale = { 100.2f, 0.2f, 0.2f };
        wall_2.m_transformComponent.rotation = { glm::radians(180.f), glm::radians(0.f), glm::radians(0.f) };

        wall_2.m_colliderComponent.SetupCollider(wall_2.m_transformComponent);
        m_gameObjects.push_back(std::move(wall_2));


        auto wall_3 = VEGameObject::CreateGameObject();
        wall_3.m_model = veModel_3;
        wall_3.m_transformComponent.translation = { 8.f, -2.f, 2.5f };
        wall_3.m_transformComponent.scale = { .2f, 100.2f, 0.2f };
        wall_3.m_transformComponent.rotation = { glm::radians(180.f), glm::radians(0.f), glm::radians(0.f) };

        wall_3.m_colliderComponent.SetupCollider(wall_3.m_transformComponent);
        m_gameObjects.push_back(std::move(wall_3));

        auto wall_4 = VEGameObject::CreateGameObject();
        wall_4.m_model = veModel_3;
        wall_4.m_transformComponent.translation = { -8.f, -2.f, 2.5f };
        wall_4.m_transformComponent.scale = { .2f, 100.2f, 0.2f };
        wall_4.m_transformComponent.rotation = { glm::radians(180.f), glm::radians(0.f), glm::radians(0.f) };

        wall_4.m_colliderComponent.SetupCollider(wall_4.m_transformComponent);
        m_gameObjects.push_back(std::move(wall_4));

        for (int row = 0; row < 12; ++row) 
        {
            for (int col = 0; col < 5; ++col) 
            {
                auto blockGameObject = VEGameObject::CreateGameObject();
                blockGameObject.m_model = veModel;
                blockGameObject.m_transformComponent.translation = { -5.5f + row * 1.0f, -6.f + col * 0.75f, 2.5f };
                blockGameObject.m_transformComponent.scale = { .30f, 0.15f, .30f };
                blockGameObject.m_transformComponent.rotation = { glm::radians(180.f), glm::radians(0.f), glm::radians(0.f) };

                blockGameObject.m_colliderComponent.SetupCollider(blockGameObject.m_transformComponent);

                m_gameObjects.push_back(std::move(blockGameObject));
            }
        }

    }
}
