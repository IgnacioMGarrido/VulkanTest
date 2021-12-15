#pragma once
#include "VEPipeline.h"
#include "VEdevice.h"
#include "VEModel.h"
#include "VEGameObject.h"
#include <memory>
#include <vector>
namespace VE
{
    class VESimpleRenderSystem
    {
    public:
        VESimpleRenderSystem(VEDevice& device, VkRenderPass renderPass);
        ~VESimpleRenderSystem();
        VESimpleRenderSystem(const VESimpleRenderSystem&) = delete;
        VESimpleRenderSystem& operator=(const VESimpleRenderSystem&) = delete;

        void RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<VEGameObject>& gameObjects);

    private:
        void CreatePipelineLayout();
        void CreatePipeline(VkRenderPass renderPass);

    private:
        VEDevice& m_device;
        std::unique_ptr<VEPipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;
    };

}