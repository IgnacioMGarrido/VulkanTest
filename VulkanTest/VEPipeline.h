#pragma once
#include <string>
#include <vector>
namespace VE {

    class VEPipeline
    {
    public:
        VEPipeline(const std::string& vertFilepath, const std::string& fragFilepath);
    
    private:
        static std::vector<char> ReadFile(const std::string& filePath);

        void createGrpahicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath);
    };
}
