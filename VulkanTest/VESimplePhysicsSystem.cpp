#include "VESimplePhysicsSystem.h"
#include <iostream>

namespace VE
{
    void VESimplePhysicsSystem::SimulateGameObjects(FrameInfo& frameInfo, std::vector<VEGameObject>& gameObjects)
    {

        for (auto& gameObject : gameObjects)
        {
            gameObject.m_colliderComponent.SetupCollider(gameObject.m_transformComponent);
            //Check collisions
            for (auto& other : gameObjects) {
                if (gameObject.GetId() != other.GetId()
                    && gameObject.m_colliderComponent.Collided(other.m_colliderComponent)) {

                    if (gameObject.GetId() == 0) // paddle game Object 
                    {

                    }
                    else if (gameObject.GetId() == 1) // ball game Object 
                    {
                        std::cout << "Ball Rebound!" << std::endl;
                        gameObject.m_rigidBodyComponent.velocity = -gameObject.m_rigidBodyComponent.velocity;
                    }
                    //else //is a block
                    //{
                    //    std::cout << "Block Destroyed!" << std::endl;
                    //    gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), gameObject), gameObjects.end());
                    //}
                }
            }

            //Update Ball
            if (gameObject.GetId() == 1) 
            { // ball game Object 
                gameObject.m_transformComponent.translation.y += frameInfo.frameTime * gameObject.m_rigidBodyComponent.velocity.y;
            }
        }


    }
}