///////////////////////////////////////////////////////////////////////////////
///
///	@File  : ParticleSystem.cpp
/// @Brief : This file implements the ParticleSystem class, responsible for 
///          managing particle effects in the engine. It handles particle 
///          emission, updating, and rendering, supporting different emission 
///          shapes and dynamic properties like velocity, lifetime, color, and 
///          texture. The system ensures efficient particle reuse and integrates 
///          with the ECS for entity-based particle behavior.
///
///	@Main Author : Edwin Leow (100%)
///	@Secondary Author : NIL
/// @Copyright 2025, Digipen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "ParticleSystem.h"
#include "Coordinator.h"
#include "InputHandler.h"

extern Framework::Coordinator ecsInterface;
namespace Framework
{
    ParticleSystem GlobalParticleSystem;

    // Get reference to the global texture assets map
    auto& textureParticleAssets = GlobalAssetManager.UE_GetAllTextureAssets();

    ParticleSystem::ParticleSystem()
    {
        // Initialize InputHandler
        InputHandlerInstance = InputHandler::GetInstance();
        particleMesh = &Graphics::getMesh("sprite");
        particles.resize(maxParticles);
    }

    void ParticleSystem::Initialize()
    {
        Signature signature;
        ecsInterface.RegisterComponent<ParticleComponent>();
        signature.set(ecsInterface.GetComponentType<ParticleComponent>());
        ecsInterface.SetSystemSignature<ParticleSystem>(signature);

        InputHandlerInstance = InputHandler::GetInstance();
        particleMesh = &Graphics::getMesh("sprite");
        particles.resize(maxParticles);
        for (ParticleComponent& p : particles)
        {
            p.active = false;
            p.life = 0.0f;
        }
    }

    void ParticleSystem::Update(float deltaTime)
    {
        if (engineState.IsPaused() || !engineState.IsPlay()) 
        {
            return;
        }

        for (auto const& entityId : mEntities)
        {
            // Conditions for how particles will be emitted
            if (ecsInterface.HasComponent<CollisionComponent>(entityId))
            {
                CollisionComponent& collision = ecsInterface.GetComponent<CollisionComponent>(entityId);
                if (collision.type == Bullet)               // Bullet Trailing Particles
                {
                    emit(entityId, deltaTime);
                }
            }

            if (ecsInterface.HasComponent<EnemyComponent>(entityId))
            {
                EnemyComponent& enemy = ecsInterface.GetComponent<EnemyComponent>(entityId);
                
                if ((enemy.type == EnemyType::Boss ||       // Only emit if collision with enemies with 0 HP, not player
                    enemy.type == EnemyType::Minion ||
                    enemy.type == EnemyType::Poison) && 
                    enemy.health <= 0)
                {
                    emit(entityId, deltaTime);
                }
            }

            if (ecsInterface.GetEntityName(entityId) == "Text")
            {
                if (abilityTest == true)
                {
                    emit(entityId, deltaTime);
                    abilityTest = false;
                }
            }
            
            /*
            check if have particle component
            emit if have
            make the particles of that entity active if conditions is met
            */
        }

        for (ParticleComponent& p : particles)
        {
            if (p.active)
            {
                // Convert world position to viewport space (Main Viewport)
                /*float viewportX = ((p.position.x - Graphics::viewportOffsetX) / Graphics::viewportWidth) * Graphics::viewportWidth + Graphics::viewportOffsetX;
                float viewportY = ((p.position.y - Graphics::viewportOffsetY) / Graphics::viewportHeight) * Graphics::viewportHeight + Graphics::viewportOffsetY;

                glm::vec2 viewportPos(viewportX, viewportY);
                glm::vec2 viewportScale(p.size * (Graphics::viewportWidth / Graphics::viewportWidth),
                    p.size * (Graphics::viewportHeight / Graphics::viewportHeight));*/

                float normalizedX = (p.position.x / Graphics::projWidth) * Graphics::viewportWidth + Graphics::viewportOffsetX;
                float normalizedY = (p.position.y / Graphics::projHeight) * Graphics::viewportHeight + Graphics::viewportOffsetY;

                glm::vec2 viewportPos(normalizedX, normalizedY);
                glm::vec2 viewportScale(p.size * (Graphics::viewportWidth / Graphics::projWidth), p.size * (Graphics::viewportHeight / Graphics::projHeight));
                
                particleMesh->textureID = GlobalAssetManager.UE_LoadTextureToOpenGL(p.textureName);
                particleMesh->modelMatrix = Graphics::calculate2DTransform(viewportPos, 0, viewportScale);
                particleMesh->alpha = p.life / 5.0f;
                particleMesh->color = p.color;
                particleMesh->draw();

                // Update particle movement
                p.position += p.velocity * deltaTime;
                p.life -= deltaTime;

                if (p.life <= 0.0f)
                {
                    p.active = false;       // Mark as inactive
                    //p.life = 0.0f;        // Reset life
                }
            }
        }
    }

    std::string ParticleSystem::GetName()
    {
        return std::string();
    }

    void ParticleSystem::emit(Entity entity, float deltaTime)
    {
        if (ecsInterface.HasComponent<ParticleComponent>(entity))
        {
            ParticleComponent& particleData = ecsInterface.GetComponent<ParticleComponent>(entity);
            
            if (ecsInterface.HasComponent<TransformComponent>(entity))
            {
                TransformComponent& transform = ecsInterface.GetComponent<TransformComponent>(entity);

                particleData.emitTimer += deltaTime;                        // Update emission timer
                
                if (particleData.emitTimer >= particleData.emitDelay)       // Only emit if enough time has passed
                {
                    glm::vec2 spawnPosition = transform.position;           // Get entity's position

                    for (unsigned int i = 0; i < particleData.emissionRate; i++)
                    {
                        ParticleComponent* p = getInactiveParticle();
                        if (p)
                        {
                            p->textureName = particleData.textureName;
                            p->position = spawnPosition;
                            p->velocity = randomVelocity(particleData.shape);
                            p->active = true;
                            p->life = particleData.life;
                            p->size = particleData.size;
                            p->color = particleData.color;
                        }
                    }
                }
            }
        }
    }

    void ParticleSystem::emitDamageNumber(Entity entity, int damage)
    {
        if (ecsInterface.HasComponent<TransformComponent>(entity))
        {
            TransformComponent& transform = ecsInterface.GetComponent<TransformComponent>(entity);
            glm::vec2 spawnPosition = transform.position;

            std::string damageStr = std::to_string(damage); // Convert damage to string

            float offsetX = 0.0f; // Offset each digit slightly

            for (char digit : damageStr)
            {
                (void)digit;
                ParticleComponent* p = getInactiveParticle();
                if (p)
                {
                    //p->textureName = "hp_" + std::string(1, digit) + ".png"; // Assign texture based on digit
                    p->textureName = "fire";
                    p->position = spawnPosition + glm::vec2(offsetX, 0); // Offset each digit
                    p->velocity = glm::vec2(0.0f, -50.0f); // Move upward
                    p->active = true;
                    p->life = 2.0f;     // Fade over 1 second
                    p->size = 50.0f;    // Adjust size if needed
                    p->color = glm::vec3(1.0f, 1.0f, 1.0f); // White
                    offsetX += 20.0f; // Adjust spacing between digits
                }
            }
        }
    }

    glm::vec2 ParticleSystem::randomVelocity(EmissionShape shape)
    {
        switch (shape)
        {
        case EmissionShape::CIRCLE:
        {
            float angle = static_cast<float>(static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159265358979323846);
            float speed = 100.0f;
            return glm::vec2(cos(angle), sin(angle)) * speed;
        }
        case EmissionShape::BOX:
        {
            float x = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * 50;
            float y = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * 50;
            return glm::vec2(x, y);
        }
        case EmissionShape::ELLIPSE: // WIP
        {
            float angle = static_cast<float>(static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159265358979323846);
            float speed = 100.0f;
            float xFactor = 1.5f;  // Horizontal stretch
            float yFactor = 1.0f;  // Vertical stretch
            return glm::vec2(cos(angle) * xFactor, sin(angle) * yFactor) * speed;
        }
        case EmissionShape::LINE:
        {
            // Here you can define a direction vector
            //glm::vec2 direction = glm::normalize(glm::vec2(1.0f, 0.0f)); // example direction (right)
            //float speed = 100.0f;
            //return direction * speed;

            // Bullet velocity should influence the particle movement, but with small randomness.
            glm::vec2 direction = glm::normalize(glm::vec2(1.0f, 0.0f)); // Bullet's forward direction (x-axis for simplicity)

            // Add randomness to simulate minor jitter
            float jitter = static_cast<float>(rand()) / RAND_MAX * 10.0f; // Random jitter (offset) along the direction

            // You can use `direction * speed` to simulate the trail speed
            float speed = 50.0f + jitter;  // Main speed + small random offset
            return direction * speed; // Particles will be emitted slightly offset from the bullet's direction
        }
        case EmissionShape::SPIRAL:
        {
            float angle = static_cast<float>(static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159265358979323846);
            float radius = static_cast<float>(rand()) / RAND_MAX * 50.0f;  // Spiral radius
            float speed = 100.0f;
            float spiralSpeed = 5.0f;  // The speed at which the spiral expands
            glm::vec2 velocity = glm::vec2(cos(angle), sin(angle)) * (radius + spiralSpeed);
            return velocity * speed;
        }
        case EmissionShape::RADIAL:
        {
            float angle = static_cast<float>(static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159265358979323846);
            float speed = 100.0f;
            float radialSpeed = 200.0f;  // Radial push speed
            (void)speed;
            return glm::vec2(cos(angle), sin(angle)) * radialSpeed;
        }
        case EmissionShape::RANDOM:
        {
            float angle = static_cast<float>(static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159265358979323846);
            float speed = static_cast<float>(rand()) / RAND_MAX * 100.0f;  // Random speed
            float radius = static_cast<float>(rand()) / RAND_MAX * 25.0f;  // Random radius
            return glm::vec2(cos(angle), sin(angle)) * speed;
            (void)radius;
            (void)speed;
        }
        case EmissionShape::WAVE:
        {
            float angle = static_cast<float>(static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159265358979323846);
            float speed = 100.0f;
            float waveFrequency = 2.0f;  // Frequency of the sine wave
            float waveAmplitude = 10.0f; // Amplitude of the sine wave
            return glm::vec2(cos(angle), sin(angle) * waveAmplitude * sin(waveFrequency * angle)) * speed;
        }
        case EmissionShape::CONE: // WIP
        {
            float angle = static_cast<float>(static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159265358979323846);
            float coneAngle = static_cast<float>(30.0f * (3.14159265358979323846 / 180.0f));    // Angle in radians
            float speed = 100.0f;
            float coneFactor = cos(coneAngle * (rand() / float(RAND_MAX)));                     // Apply the cone angle
            return glm::vec2(cos(angle) * coneFactor, sin(angle) * coneFactor) * speed;
        }
        case EmissionShape::EXPLOSION:
        {
            float angle = static_cast<float>(static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159265358979323846);
            float speed = 100.0f;
            float burstSpeed = 500.0f;      // Stronger initial burst
            (void)speed;
            return glm::vec2(cos(angle), sin(angle)) * burstSpeed;
        }
        default:
            return glm::vec2(0.0f, 0.0f);
        }
    }

    void ParticleSystem::resetParticles(Entity entity, std::string textureName)
    {
        if (ecsInterface.HasComponent<ParticleComponent>(entity))
        {
            ParticleComponent& particleComponent = ecsInterface.GetComponent<ParticleComponent>(entity);

            particleComponent.textureName = textureName;

            // Refresh the texture ID
            particleMesh->textureID = GlobalAssetManager.UE_LoadTextureToOpenGL(textureName);
            glBindTexture(GL_TEXTURE_2D, particleMesh->textureID);

            //std::cout << "new particle texture id " << particleMesh->textureID << " with new texture " << particleComponent.textureName << std::endl;

            particles.clear();
            particles.resize(maxParticles);

            // Reset each particle
            for (auto& particle : particles)
            {
                particle.textureName = textureName;
                particle.position = particleComponent.position;
                particle.velocity = particleComponent.velocity;
                particle.color = particleComponent.color;
                particle.size = particleComponent.size;
                particle.life = particleComponent.life;
                particle.active = false;                        // Mark them inactive to be re-emitted
            }
        }
    }

    ParticleComponent* ParticleSystem::getInactiveParticle()
    {
        for (ParticleComponent& p : ParticleSystem::particles)	            // Iterate through all particles
        {
            if (!p.active)							                        // Check if active or not
            {
                return &p;							                        // Return reference to that particle
            }
        }
        return nullptr;			                                            // No available inactive particles
    }

    glm::vec2 ParticleSystem::randomVelocity()
    {
        return glm::vec2							                
        {
            (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * 50,     // Return vector of (x,y)
            (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * 50
        };
    }
}