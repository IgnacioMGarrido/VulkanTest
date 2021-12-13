#pragma once
#define GLFW_INCLUDE_WINDOW
#include <GLFW/glfw3.h>
#include <string>
namespace VE 
{
    class VEwindow 
    {
    public:
        VEwindow(std::string name = "Vulkan Experiment", int width = 800, int height = 600);
        VEwindow(const VEwindow&) = delete;
        VEwindow& operator=(const VEwindow&) = delete;
        ~VEwindow();
    
        bool ShouldClose();
        void RetreivePollEvents();
    private:
        void InitWindow();

    private:
        std::string m_name;
        const int m_width;
        const int m_height;
        GLFWwindow* m_window = nullptr;
    };
}
