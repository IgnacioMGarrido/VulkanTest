#include "VEWindow.h"
#include <stdexcept>
namespace VE 
{
    VEwindow::VEwindow(int width, int height, std::string name)
        : m_name {name}
        , m_width {width}
        , m_height{ height }
    {
        InitWindow();
    }

    VEwindow::~VEwindow()
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    void VEwindow::FrameBufferResizedCallback(GLFWwindow* window, int width, int height)
    {
        auto veWindow = reinterpret_cast<VEwindow*>(glfwGetWindowUserPointer(window));
        veWindow->m_frameBufferResized = true;
        veWindow->m_width = width;
        veWindow->m_height = height;
    }

    void VEwindow::InitWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
        m_window = glfwCreateWindow(m_width, m_height, m_name.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, FrameBufferResizedCallback);
    }

    void VEwindow::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
    {
        if (glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS) 
        {
            throw std::runtime_error("Failed to Create widnow Surface");
        }
    }

    VkExtent2D VEwindow::GetExtent()
    {
        return {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height)};
    }

    bool VEwindow::ShouldClose()
    {
        return glfwWindowShouldClose(m_window);
    }

    void VEwindow::RetreivePollEvents()
    {
        glfwPollEvents();
    }

}

