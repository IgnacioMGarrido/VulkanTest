#include "VEPipeline.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
namespace VE 
{
    VEPipeline::VEPipeline(const std::string& vertFilepath, const std::string& fragFilepath)
    {
        createGrpahicsPipeline(vertFilepath, fragFilepath);
    }

    std::vector<char> VEPipeline::ReadFile(const std::string& filePath)
    {
        std::ifstream file{ filePath, std::ios::ate | std::ios::binary };

        if (!file.is_open()) 
        {
            throw std::runtime_error("File to open file: " + filePath);
        }

        size_t fileSize = static_cast<size_t>(file.tellg());

        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }

    void VEPipeline::createGrpahicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath)
    {
        auto vertCode = ReadFile(vertFilepath);
        auto fragCode = ReadFile(fragFilepath);

        std::cout << "vertex Shader Code Size: " << vertCode.size() << '\n';
        std::cout << "fragment Shader Code Size: " << fragCode.size() << '\n';
    }
}