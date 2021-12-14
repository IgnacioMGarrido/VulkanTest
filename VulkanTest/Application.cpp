#include "Application.h"
#include <stdexcept>
namespace VE 
{
    Application::Application()
    {
        CreatePipelineLayout();
        CreatePipeline();
        CreateCommandBuffers();
    }
    
    Application::~Application()
    {
        vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
    }

    void Application::Run()
    {
        while (!m_levelWindow.ShouldClose()) 
        {
            m_levelWindow.RetreivePollEvents();
        }
    
    }
    
    void Application::CreatePipelineLayout()
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create Pipeline Layout");
        }
    }

    void Application::CreatePipeline()
    {
        auto pipelineConfig = VEPipeline::DefaultPipelineConfigInfo(m_SwapChain.width(), m_SwapChain.height());

        pipelineConfig.renderPass = m_SwapChain.getRenderPass();
        pipelineConfig.pipelineLayout = m_pipelineLayout;
        m_pipeline = std::make_unique<VEPipeline>(m_device, "Shaders/basic_shader.vert.spv", "Shaders/basic_shader.frag.spv", pipelineConfig);
    }
    
    void Application::CreateCommandBuffers()
    {
    }
    
    void Application::DrawFrame()
    {
    }
}
