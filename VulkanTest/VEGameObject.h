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

        glm::mat4 mat4() 
        {
            auto transform = glm::translate(glm::mat4{1.f},  translation);

            transform = glm::rotate(transform, rotation.y, { 0.f, 1.f, 0.f});
            transform = glm::rotate(transform, rotation.x, { 1.f, 0.f, 0.f });
            transform = glm::rotate(transform, rotation.z, { 0.f, 0.f, 1.f });

            transform = glm::scale(transform, scale);
            return transform;
        }
    };
    class VEGameObject
    {
    public:
        using id_t = unsigned int;

        VEGameObject(const VEGameObject&) = delete;
        VEGameObject& operator=(const VEGameObject&) = delete;

        VEGameObject(VEGameObject&&) = default;
        VEGameObject& operator=(VEGameObject&&) = default;


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

    private:
        VEGameObject(id_t objId) : m_id(objId) {};

    private:
        id_t m_id;

    };
}
