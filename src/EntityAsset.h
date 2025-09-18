///////////////////////////////////////////////////////////////////////////////
///
///	@File  : EntityAsset.h
/// @Brief : Defines the EntityAsset class, responsible for loading and managing
///          entity configurations for the application. This includes deserializing 
///          entity data from files and integrating with the ECS (Entity Component System).
///
///	@Main Author : Edwin Leow (100%)
/// @Secondary Author : NIL
///	@Copyright 2024, Digipen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef _ENTITY_H_
#define _ENTITY_H_
#include "pch.h"
#include "JsonSerialize.h"
#include "Coordinator.h"
#include "ComponentList.h"

extern Framework::Coordinator ecsInterface;

/**
 * @class EntityAsset
 * @brief Manages the loading and configuration of entities within the ECS, allowing
 *        for entity creation and setup from configuration files.
 */
class EntityAsset
{
public:

    struct Animation
    {
        int rows;
        int cols;
        float animationSpeed;
    };

    struct BulletData
    {
        glm::vec2 scale{ 0,0 };
        std::string textureID = "noTexture";
        glm::vec3 color{ 0,0,0 };
        float alpha = 1;
        glm::vec2 baseVelocity{ 0,0 };
        std::string fontName = "noFont";
        std::string particleTexture = "noParticleTexture";
        float particleLife = 1.f;
        float particleSize = 1.f;
        glm::vec3 particleColor{ 0,0,0 };
        float emitDelay = 1.f;
        float emissionRate = 1.f;
        int damageMultiplier = 1;
        glm::vec2 collisionScale{ 10,10 };
    };

    EntityAsset();

    /**
     * @brief Constructs an EntityAsset and loads entity configurations from a file.
     * @param filePath Path to the file containing entity data.
     */
    EntityAsset(const std::string& filePath, glm::vec2 newPosition = glm::vec2(-1, -1));
    
    /**
     * @brief Destructor for the EntityAsset class.
     */
    ~EntityAsset();
	
    /**
     * @brief Deserializes and loads entity configurations from a specified file.
     * @param filePath Path to the file containing entity data.
     * #param Default Position to toggle between dynamic spawning for prefabs or refer from json.
     */
    void DeserializeEntities(const std::string& filename, glm::vec2 newPosition = glm::vec2(-1, -1));

    void SerializeEntities(const std::string& filePath);

    void DeserializeAnimation(const std::string& filePath);

    void DeserializeBullet(const std::string& filePath);

    std::string EnemyTypeToString(EnemyType type);

    std::string ObjectTypeToString(ObjectType type);

private:
};

extern EntityAsset GlobalEntityAsset;

#endif // !_ENTITY_H_