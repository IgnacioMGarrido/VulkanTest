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
    
        const glm::mat4& GetProjection() const { return m_projectionMatrix; };
    private:
        glm::mat4 m_projectionMatrix{ 1.f };
    };
}

