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

    class GravityPhysicsSystem
    {
    public:
        GravityPhysicsSystem(float strength) : strengthGravity{ strength } {}

        const float strengthGravity;

        void update(std::vector<VEGameObject>& objs, float dt, unsigned int substeps = 1) 
        {
            const float stepDelta = dt / substeps;
            for (int i = 0; i < substeps; i++) 
            {
                stepSimulation(objs, stepDelta);
            }
        }

        glm::vec2 computeForce(VEGameObject& fromObj, VEGameObject& toObj) const {
            auto offset = fromObj.m_transformComponent.m_translation - toObj.m_transformComponent.m_translation;
            float distanceSquared = glm::dot(offset, offset);

            // Just going to return 0 if objects are too close together...
            if (glm::abs(distanceSquared) < 1e-10f)
            {
                return { .0f, .0f };
            }

            float force =
                strengthGravity * toObj.m_rigidBodyComponent.mass * fromObj.m_rigidBodyComponent.mass / distanceSquared;
            return force * offset / glm::sqrt(distanceSquared);
        }

    private:
        void stepSimulation(std::vector<VEGameObject>& physicsObjs, float dt)
        {
            // Loops through all pairs of objects and applies attractive force between them
            for (auto iterA = physicsObjs.begin(); iterA != physicsObjs.end(); ++iterA)
            {
                auto& objA = *iterA;
                for (auto iterB = iterA; iterB != physicsObjs.end(); ++iterB)
                {
                    if (iterA == iterB) continue;
                    auto& objB = *iterB;

                    auto force = computeForce(objA, objB);
                    objA.m_rigidBodyComponent.velocity += dt * -force / objA.m_rigidBodyComponent.mass;
                    objB.m_rigidBodyComponent.velocity += dt * force / objB.m_rigidBodyComponent.mass;
                }
            }

            // update each objects position based on its final velocity
            for (auto& obj : physicsObjs) 
            {
                obj.m_transformComponent.m_translation += dt * obj.m_rigidBodyComponent.velocity;
            }
        }
    };

    class Vec2FieldSystem
    {
    public:
        void update(
            const GravityPhysicsSystem& physicsSystem,
            std::vector<VEGameObject>& physicsObjs,
            std::vector<VEGameObject>& vectorField) 
        {
            // For each field line we caluclate the net graviation force for that point in space
            for (auto& vf : vectorField) 
            {
                glm::vec2 direction{};
                for (auto& obj : physicsObjs) 
                {
                    direction += physicsSystem.computeForce(obj, vf);
                }

                // This scales the length of the field line based on the log of the length
                // values were chosen just through trial and error based on what i liked the look
                // of and then the field line is rotated to point in the direction of the field
                vf.m_transformComponent.m_scale.x =
                    0.005f + 0.045f * glm::clamp(glm::log(glm::length(direction) + 1) / 3.f, 0.f, 1.f);
                vf.m_transformComponent.m_rotation = atan2(direction.y, direction.x);
            }
        }
    };

    std::unique_ptr<VEModel> createSquareModel(VEDevice& device, glm::vec2 offset)
    {
        std::vector<VEModel::Vertex> vertices = 
        {
            {{-0.5f, -0.5f}},
            {{0.5f, 0.5f}},
            {{-0.5f, 0.5f}},
            {{-0.5f, -0.5f}},
            {{0.5f, -0.5f}},
            {{0.5f, 0.5f}},  //
        };
        for (auto& v : vertices)
        {
            v.position += offset;
        }
        return std::make_unique<VEModel>(device, vertices);
    }

    std::unique_ptr<VEModel> createCircleModel(VEDevice& device, unsigned int numSides)
    {
        std::vector<VEModel::Vertex> uniqueVertices{};
        for (int i = 0; i < numSides; i++)
        {
            float angle = i * glm::two_pi<float>() / numSides;
            uniqueVertices.push_back({ {glm::cos(angle), glm::sin(angle)} });
        }
        uniqueVertices.push_back({});  // adds center vertex at 0, 0

        std::vector<VEModel::Vertex> vertices{};
        for (int i = 0; i < numSides; i++)
        {
            vertices.push_back(uniqueVertices[i]);
            vertices.push_back(uniqueVertices[(i + 1) % numSides]);
            vertices.push_back(uniqueVertices[numSides]);
        }
        return std::make_unique<VEModel>(device, vertices);
    }

    //Application Body.
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
        // create some models
        std::shared_ptr<VEModel> squareModel = createSquareModel(
            m_device,
            { .5f, .0f });  // offset model by .5 so rotation occurs at edge rather than center of square
        std::shared_ptr<VEModel> circleModel = createCircleModel(m_device, 64);

        // create physics objects
        std::vector<VEGameObject> physicsObjects{};
        auto red = VEGameObject::CreateGameObject();
        red.m_transformComponent.m_scale = glm::vec2{ .05f };
        red.m_transformComponent.m_translation = { .5f, .5f };
        red.m_color = { 1.f, 0.f, 0.f };
        red.m_rigidBodyComponent.velocity = { -.5f, .0f };
        red.m_model = circleModel;
        physicsObjects.push_back(std::move(red));
        auto blue = VEGameObject::CreateGameObject();
        blue.m_transformComponent.m_scale = glm::vec2{ .05f };
        blue.m_transformComponent.m_translation = { -.45f, -.25f };
        blue.m_color = { 0.f, 0.f, 1.f };
        blue.m_rigidBodyComponent.velocity = { .5f, .0f };
        blue.m_model = circleModel;
        physicsObjects.push_back(std::move(blue));

        // create vector field
        std::vector<VEGameObject> vectorField{};
        int gridCount = 40;
        for (int i = 0; i < gridCount; i++)
        {
            for (int j = 0; j < gridCount; j++)
            {
                auto vf = VEGameObject::CreateGameObject();
                vf.m_transformComponent.m_scale = glm::vec2(0.005f);
                vf.m_transformComponent.m_translation =
                {
                    -1.0f + (i + 0.5f) * 2.0f / gridCount,
                    -1.0f + (j + 0.5f) * 2.0f / gridCount
                };
                vf.m_color = glm::vec3(1.0f);
                vf.m_model = squareModel;
                vectorField.push_back(std::move(vf));
            }
        }

        GravityPhysicsSystem gravitySystem{ 0.81f };
        Vec2FieldSystem vecFieldSystem{};

        VESimpleRenderSystem simpleRenderSystem{ m_device, m_renderer.GetSwapChainRenderPass() };

        while (!m_levelWindow.ShouldClose())
        {
            m_levelWindow.RetreivePollEvents();
            if (auto commandBuffer = m_renderer.BeginFrame())
            {
                // update systems
                gravitySystem.update(physicsObjects, 1.f / 60, 5);
                vecFieldSystem.update(gravitySystem, physicsObjects, vectorField);

                //This way we can add multiple render passes (Shadows, reflection, etc)
                m_renderer.BeginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.RenderGameObjects(commandBuffer, physicsObjects);
                simpleRenderSystem.RenderGameObjects(commandBuffer, vectorField);
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
        triangle.m_transformComponent.m_translation.x = .0f;
        triangle.m_transformComponent.m_scale = { 1.0f, 1.0f };
        triangle.m_transformComponent.m_rotation = .25f * glm::two_pi<float>();

        m_gameObjects.push_back(std::move(triangle));
    }
}
