#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace VE 
{
    class VEwindow 
    {
    public:
        VEwindow(int width = 800, int height = 600, std::string name = "Vulkan Experiment");
        VEwindow(const VEwindow&) = delete;
        VEwindow& operator=(const VEwindow&) = delete;
        ~VEwindow();
    
        bool ShouldClose();
        bool WasWindowResized() { return m_frameBufferResized; };
        void ResetWindowResizedFlag() { m_frameBufferResized = false; }

        //TODO: Abstract this so no dependencies on GLFW.
        GLFWwindow* GetGLFWwindow() const { return m_window; }

        void RetreivePollEvents();
        void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
        VkExtent2D GetExtent();
    private:
        static void FrameBufferResizedCallback(GLFWwindow* window, int width, int height);
        void InitWindow();
    private:
        std::string m_name;
        int m_width;
        int m_height;
        bool m_frameBufferResized = false;
        GLFWwindow* m_window = nullptr;
    };
}
