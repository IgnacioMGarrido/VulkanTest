#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_ONE
#include <glm/glm.hpp>
namespace VE 
{
    class VECamera
    {
    public:
        void SetOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
        void SetPerspectiveProjection(float fovy, float aspect, float near, float far);
    
        void SetViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{ 0.0f, -1.0f, 0.0f });
        void SetViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{ 0.0f, -1.0f, 0.0f });
        void SetViewYXZ(glm::vec3 position, glm::vec3 rotation);

        const glm::mat4& GetProjection() const { return m_projectionMatrix; };
        const glm::mat4& GetView() const { return m_viewMatrix; };

    private:
        glm::mat4 m_projectionMatrix{ 1.f };
        glm::mat4 m_viewMatrix{ 1.f };
    };
}

