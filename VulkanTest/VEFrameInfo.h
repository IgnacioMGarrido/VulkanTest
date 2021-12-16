#pragma once


#include "VECamera.h"
#include <vulkan/vulkan.h>
namespace VE
{
    struct FrameInfo 
    {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        VECamera &camera;
        VkDescriptorSet globalDescriptorSet;
    };
};