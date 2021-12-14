#pragma once

#include "VEModel.h"
#include <memory>

namespace VE 
{

    struct Transform2dComponent 
    {
        glm::vec2 m_translataion{};

        glm::mat2 mat2() { return glm::mat2{ 1.0f }; }
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
        Transform2dComponent m_transformComponent{};

    private:
        VEGameObject(id_t objId) : m_id(objId) {};

    private:
        id_t m_id;

    };
}
