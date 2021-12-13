#include "Application.h"

namespace VE 
{
    void Application::Run()
    {
        while (!m_levelWindow.ShouldClose()) 
        {
            m_levelWindow.RetreivePollEvents();
        }
    
    }
}
