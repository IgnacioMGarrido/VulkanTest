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
        void SimulateGameObjects(FrameInfo& frameInfo, std::vector<VEGameObject>& gameObjects);
    };
}

