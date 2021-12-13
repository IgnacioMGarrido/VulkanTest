#include "VEWindow.h"

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

    void VEwindow::InitWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
        m_window = glfwCreateWindow(m_width, m_height, m_name.c_str(), nullptr, nullptr);
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

