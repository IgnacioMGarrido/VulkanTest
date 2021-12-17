#include "Input.h"

namespace VE 
{
    //TODO: Depends on GLFW so not too good we should abstract this dependency to support other window systems
    void KeyboardMovementInput::MoveInPlaneXZ(GLFWwindow* window, float dt, VEGameObject& gameObject)
    {
        glm::vec3 rotate{ 0.f };

        if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1;
        if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1;
        if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1;
        if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1;
        

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) 
        {
            gameObject.m_transformComponent.rotation += m_lookSpeed * dt * glm::normalize(rotate);
        }

        gameObject.m_transformComponent.rotation.x = glm::clamp(gameObject.m_transformComponent.rotation.x, -1.5f, 1.5f);
        gameObject.m_transformComponent.rotation.y = glm::mod(gameObject.m_transformComponent.rotation.y, glm::two_pi<float>());

        float yaw = gameObject.m_transformComponent.rotation.y;
        const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
        const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
        const glm::vec3 upDir{ 0.f, -1.f, 0.f };

        glm::vec3 moveDir{ 0.f };
        if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
        if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
        if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
        if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
        if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
        if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS)  moveDir -= upDir;

        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
        {
            gameObject.m_transformComponent.translation += m_moveSpeed * dt * glm::normalize(moveDir);
        }
    }

    void KeyboardMovementInput::MoveLeftRight(GLFWwindow* window, float dt, VEGameObject& gameObject)
    {
        float yaw = gameObject.m_transformComponent.rotation.y;
        const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
        const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
        glm::vec3 moveDir{ 0.f };
        if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
        if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
        {
            gameObject.m_transformComponent.translation += m_moveSpeed * dt * glm::normalize(moveDir);
        }
    }


}
