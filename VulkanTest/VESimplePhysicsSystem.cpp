#include "VESimplePhysicsSystem.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>
namespace VE
{
    VESimplePhysicsSystem::VESimplePhysicsSystem()
    {
        srand(time(NULL));
    }
    void VESimplePhysicsSystem::SimulateGameObjects(FrameInfo& frameInfo, std::vector<VEGameObject>& gameObjects)
    {

        for (auto& gameObject : gameObjects)
        {
            gameObject.m_colliderComponent.SetupCollider(gameObject.m_transformComponent);
            //Check collisions
            for (auto& other : gameObjects) {
                if (gameObject.GetId() != other.GetId()
                    && gameObject.m_colliderComponent.Collided(other.m_colliderComponent)) {

                    if (gameObject.GetId() == 1) // ball game Object 
                    {
                        srand(time(NULL));
                        gameObject.m_rigidBodyComponent.velocity.y = -gameObject.m_rigidBodyComponent.velocity.y;
                        gameObject.m_rigidBodyComponent.velocity.x = static_cast<float>(rand() % (10 - (-10) + 1) + (-10)) / 10.f;
                    }
                    else if(gameObject.GetId() > 5)//Block Game Object
                    {
                        gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), gameObject), gameObjects.end());
                    }
                }
            }

            //Update Ball
            if (gameObject.GetId() == 1) 
            { // ball game Object 
                gameObject.m_transformComponent.translation.y += frameInfo.frameTime * gameObject.m_rigidBodyComponent.velocity.y;
                gameObject.m_transformComponent.translation.x += frameInfo.frameTime * gameObject.m_rigidBodyComponent.velocity.x;
                gameObject.m_transformComponent.rotation.x += frameInfo.frameTime * gameObject.m_rigidBodyComponent.velocity.x * 4;
                gameObject.m_transformComponent.rotation.y += frameInfo.frameTime * gameObject.m_rigidBodyComponent.velocity.x * 2;
                gameObject.m_transformComponent.rotation.z += frameInfo.frameTime * -gameObject.m_rigidBodyComponent.velocity.x * 3;

            }
        }


    }
}