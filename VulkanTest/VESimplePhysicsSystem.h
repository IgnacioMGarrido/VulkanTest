#pragma once

#include "VEGameObject.h"
#include "VEFrameInfo.h"
#include <memory>
#include <vector>
namespace VE 
{
    class VESimplePhysicsSystem
    {
    public:
        VESimplePhysicsSystem();
        void SimulateGameObjects(FrameInfo& frameInfo, std::vector<VEGameObject>& gameObjects);
    };
}

