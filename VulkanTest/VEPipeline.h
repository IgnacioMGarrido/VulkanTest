#pragma once
#include "VEdevice.h"

#include <string>
#include <vector>
namespace VE {

    struct PipelineConfigInfo 
    {
    
    };

    class VEPipeline
    {
    public:
        VEPipeline(
            VEDevice& device,
            const std::string& vertFilepath,
            const std::string& fragFilepath,
            const PipelineConfigInfo& info
        );

        VEPipeline(const VEPipeline&) = delete;
        VEPipeline& operator=(const VEPipeline&) = delete;

        static PipelineConfigInfo DefaultPipelineConfigInfo(uint32_t width, uint32_t height);

        ~VEPipeline() {};
    
    private:
        static std::vector<char> ReadFile(const std::string& filePath);

        void createGrpahicsPipeline(
            const std::string& vertFilepath, 
            const std::string& fragFilepath,
            const PipelineConfigInfo& info
        );

        void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

        VEDevice& m_device;
        VkPipeline m_graphicsPipeline;
        VkShaderModule m_vertShaderModuel;
        VkShaderModule m_fragShaderModuel;
    };
}
