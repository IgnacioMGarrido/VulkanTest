#pragma once

#include "VEModel.h"
#include <glm/gtc/matrix_transform.hpp>

#include <memory>

namespace VE 
{

    struct TransformComponent 
    {
        glm::vec3 translation{};
        glm::vec3 scale{1.f , 1.f, 1.f};
        glm::vec3 rotation;

        //Translate * Ry * Rx * Rz * Scale
        //Trait-bryan angles Y(1) X(2) Z(3)
        glm::mat4 mat4();
        glm::mat3 normalMatrix();
    };

    struct ColliderComponent 
    {
        float x;
        float y;
        float width;
        float height;

        ColliderComponent()
            : x(-10000.f)
            , y(-10000.f)
            , width(0.f)
            , height(0.f)
        {};

        bool operator==(const ColliderComponent& rhs) const 
        {
            return x == rhs.x && y == rhs.y && width == rhs.width && height == rhs.height;
        }
        
        void SetupCollider(const TransformComponent& transformComponent) 
        {
            width = transformComponent.scale.x + 0.5f;
            height = transformComponent.scale.y;
            x = transformComponent.translation.x - width / 2;
            y = transformComponent.translation.y - height / 2;
        }

        bool Collided(const ColliderComponent& other) 
        {
            if (other == *this) 
            {
                return false;
            }
            if (x < other.x + other.width &&
                x + width > other.x &&
                y < other.y + other.height &&
                height + y > other.y) {
                // collision detected!
                return true;
            }
            else {
                // no collision
                return false;
            }
        }

    };

    struct RigidBody2dComponent
    {
        glm::vec2 velocity;
        float mass{ 1.0f };
    };

    class VEGameObject
    {
    public:
        using id_t = unsigned int;

        VEGameObject(const VEGameObject&) = delete;
        VEGameObject& operator=(const VEGameObject&) = delete;

        VEGameObject(VEGameObject&&) = default;
        VEGameObject& operator=(VEGameObject&&) = default;
        bool operator==(const VEGameObject& rhs) const { return m_id == rhs.m_id; }

        static VEGameObject CreateGameObject() 
        {
            static id_t currentId = 0;
            return VEGameObject(currentId++);
        }

        id_t GetId() { return m_id; };

    public:
        std::shared_ptr<VEModel> m_model{};
        glm::vec3 m_color{};
        TransformComponent m_transformComponent{};
        ColliderComponent m_colliderComponent{};
        RigidBody2dComponent m_rigidBodyComponent{};

    private:
        VEGameObject(id_t objId) : m_id(objId) {};

    private:
        id_t m_id;

    };
}
