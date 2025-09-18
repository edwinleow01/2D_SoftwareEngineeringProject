///////////////////////////////////////////////////////////////////////////////
///
///	@File  : EntityAsset.cpp
/// @Brief : This file defines the EntityAsset class, which is responsible for 
///          loading and managing entity configurations for the application. 
///          The class deserializes entity data from JSON files and integrates 
///          them into the Entity-Component System (ECS), allowing the application 
///          to load and organize entities with their associated components, 
///          such as transformations, rendering properties, and behavior functions.
///
///	@Main Author : Edwin Leow (70%)
/// @Secondary Authors : Joshua (15%), Dylan (15%)
///	@Copyright 2024, Digipen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "EntityAsset.h"
#include "LogicManager.h"
#include "AssetManager.h"
#include "Vector2D.h"
#include "Coordinator.h"

EntityAsset GlobalEntityAsset;

EntityAsset::EntityAsset() 
{
    DeserializeBullet("Assets/JsonData/BulletAsset.json");
    DeserializeAnimation("Assets/JsonData/AnimationAsset.json");
}

EntityAsset::EntityAsset(const std::string& filePath, glm::vec2 newPosition)
{
    DeserializeEntities(filePath, newPosition);
}

EntityAsset::~EntityAsset() {}

void EntityAsset::DeserializeEntities(const std::string& filename, glm::vec2 newPosition)   
{
    // Read JSON file
    std::ifstream ifs(filename);
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document document;
    document.ParseStream(isw);

    // Check for errors in parsing
    if (document.HasParseError())
    {
        std::cerr << "Error parsing JSON file!" << std::endl;
        return;
    }

    // Check if "entities" array exists
    if (!document.HasMember("entities") || !document["entities"].IsArray())
    {
        std::cerr << "Invalid or missing 'entities' array!" << std::endl;
        return;
    }

    // Iterate over each entity in the "entities" array
    const rapidjson::Value& entities = document["entities"];
    for (rapidjson::SizeType i = 0; i < entities.Size(); ++i)
    {
        const rapidjson::Value& entity = entities[i];

        // Check for the "type" field
        if (!entity.HasMember("type") || !entity["type"].IsString())
        {
            std::cerr << "Entity missing 'type' field or 'type' is not a string!" << std::endl;
            continue;
        }

        // Get the entity type (name)
        std::string entityType = entity["type"].GetString();

        // Create a new entity
        Framework::Entity newEntity = ecsInterface.CreateEntity();

        ecsInterface.SetEntityName(newEntity, entityType); // Assuming you have a function to set entity name

        // Check if the entity has components
        if (entity.HasMember("components") && entity["components"].IsObject())
        {
            const rapidjson::Value& components = entity["components"];

            // Check and add TransformComponent
            if (components.HasMember("TransformComponent") && components["TransformComponent"].IsObject())
            {
                const rapidjson::Value& transform = components["TransformComponent"];
                TransformComponent transformComponent;
                

                // Dynamic transform for prefab. 

                if (transform.HasMember("x"))
                    transformComponent.position.x = transform["x"].GetFloat();
                if (transform.HasMember("y"))
                    transformComponent.position.y = transform["y"].GetFloat();

                // Override with new position if valid
                if (newPosition.x != -1 && newPosition.y != -1)
                {
                    transformComponent.position = newPosition;
                    std::cout << "Overriding position to: (" << newPosition.x << ", " << newPosition.y << ")\n";
                }
                else
                {
                    std::cout << "Using original position from JSON: ("
                        << transformComponent.position.x << ", "
                        << transformComponent.position.y << ")\n";
                }

                // Handle scale
                if (transform.HasMember("scaleX"))
                    transformComponent.scale.x = transform["scaleX"].GetFloat();
                if (transform.HasMember("scaleY"))
                    transformComponent.scale.y = transform["scaleY"].GetFloat();

                // Handle rotation
                if (transform.HasMember("rotation"))
                    transformComponent.rotation = transform["rotation"].GetFloat();

               
                if (transform.HasMember("tag") && transform["tag"].IsString())
                {
                    std::string tagString = transform["tag"].GetString();

                    // **Step 1: Remove all spaces**
                    tagString.erase(remove_if(tagString.begin(), tagString.end(), ::isspace), tagString.end());

                    // **Step 2: Split tags by comma**
                    std::stringstream ss(tagString);
                    std::string tag;

                    while (getline(ss, tag, ','))  // Split by comma
                    {
                        if (!tag.empty()) // Avoid adding empty tags
                        {
                            ecsInterface.AddTag(newEntity, tag);
                            std::cout << "Added tag: " << tag << std::endl;
                        }
                    }
                }
                else
                {
                    std::string defaultTag = "Entity_" + std::to_string(newEntity); // Example: "Entity_123"
                    transformComponent.tag = defaultTag;
                    ecsInterface.AddTag(newEntity, defaultTag);
                    std::cout << "Added default tag: " << defaultTag << std::endl;
                }

                ecsInterface.AddComponent<TransformComponent>(newEntity, transformComponent);
                //std::cout << "ADDED TRANSFORM COMPONENT\n";
            }

            // Check and add RenderComponent
            if (components.HasMember("RenderComponent") && components["RenderComponent"].IsObject()) {
                const rapidjson::Value& render = components["RenderComponent"];
                RenderComponent renderComponent;

                if (render.HasMember("textureID")) renderComponent.textureID = render["textureID"].GetString();

                // Parse color array
                if (render.HasMember("color") && render["color"].IsArray()) {
                    const rapidjson::Value& colorArray = render["color"];
                    if (colorArray.Size() == 3) {
                        renderComponent.color = glm::vec3(
                            colorArray[0].GetFloat(),
                            colorArray[1].GetFloat(),
                            colorArray[2].GetFloat()
                        );
                    }
                }

                if (render.HasMember("alpha")) renderComponent.alpha = render["alpha"].GetFloat();

                // Parse renderType
                if (render.HasMember("renderType") && render["renderType"].IsString()) {
                    std::string typeStr = render["renderType"].GetString();
                    if (typeStr == "Sprite") renderComponent.renderType = RenderType::Sprite;
                    else if (typeStr == "Particle") renderComponent.renderType = RenderType::Particle;
                    else if (typeStr == "Text") renderComponent.renderType = RenderType::Text;
                    else if (typeStr == "PauseUI") renderComponent.renderType = RenderType::PauseUI;
                }

                // Parse isActive
                if (render.HasMember("isActive") && render["isActive"].IsBool()) {
                    renderComponent.isActive = render["isActive"].GetBool();
                }

                ecsInterface.AddComponent<RenderComponent>(newEntity, renderComponent);
            }

            // Check and add LayerComponent
            if (components.HasMember("LayerComponent") && components["LayerComponent"].IsObject())
            {
                const rapidjson::Value& layer = components["LayerComponent"];
                LayerComponent layerComponent;

                // Check if LayerID is a string and map to enum
                if (layer.HasMember("LayerID"))
                {
                    if (layer["LayerID"].IsString())
                    {
                        std::string layerStr = layer["LayerID"].GetString();
                        if (layerStr == "Background") layerComponent.layerID = Layer::Background;
                        else if (layerStr == "Character") layerComponent.layerID = Layer::Character;
                        else if (layerStr == "Foreground") layerComponent.layerID = Layer::Foreground;
                        else if (layerStr == "UI") layerComponent.layerID = Layer::UI;
                        else if (layerStr == "Debug") layerComponent.layerID = Layer::Debug;
                        else layerComponent.layerID = Layer::Background; // Default or unknown
                    }
                    // Check if LayerID is an integer and assign directly
                    else if (layer["LayerID"].IsInt())
                    {
                        int layerInt = layer["LayerID"].GetInt();
                        // Ensure the integer is within the valid enum range
                        if (layerInt >= static_cast<int>(Layer::Background) && layerInt <= static_cast<int>(Layer::Debug))
                        {
                            layerComponent.layerID = static_cast<Layer>(layerInt);
                        }
                        else
                        {
                            layerComponent.layerID = Layer::Background; // Default if out of range
                            std::cerr << "Warning: LayerID out of range. Defaulting to Background layer.\n";
                        }
                    }
                }

                // Assign SortID if present
                if (layer.HasMember("SortID") && layer["SortID"].IsUint())
                {
                    layerComponent.sortID = layer["SortID"].GetUint();
                }

                ecsInterface.AddComponent<LayerComponent>(newEntity, layerComponent);
                // std::cout << "ADDED LAYER COMPONENT\n";
            }

            // Check and add TextComponent
            if (components.HasMember("TextComponent") && components["TextComponent"].IsObject())
            {
                const rapidjson::Value& textComp = components["TextComponent"];
                TextComponent textComponent;

                // Read "text" field
                if (textComp.HasMember("text") && textComp["text"].IsString())
                {
                    textComponent.text = textComp["text"].GetString();
                }

                // Read "fontSize" field
                if (textComp.HasMember("fontSize") && textComp["fontSize"].IsFloat() || textComp["fontSize"].IsInt())
                {
                    textComponent.fontSize = textComp["fontSize"].GetFloat();
                }

                // Read "color" array
                if (textComp.HasMember("color") && textComp["color"].IsArray())
                {
                    const rapidjson::Value& colorArray = textComp["color"];
                    if (colorArray.Size() == 3)
                    {
                        textComponent.color = glm::vec3
                        (
                            colorArray[0].GetFloat(),
                            colorArray[1].GetFloat(),
                            colorArray[2].GetFloat()
                        );
                    }
                }

                // Read "fontName" field
                if (textComp.HasMember("fontName") && textComp["fontName"].IsString())
                {
                    textComponent.fontName = textComp["fontName"].GetString();
                }

                // Read "offset" array
                if (textComp.HasMember("offset") && textComp["offset"].IsArray())
                {
                    const rapidjson::Value& offsetArray = textComp["offset"];
                    if (offsetArray.Size() == 2)
                    {
                        textComponent.offset = glm::vec2
                        (
                            offsetArray[0].GetFloat(),
                            offsetArray[1].GetFloat()
                        );
                    }
                }

                // Add TextComponent to entity
                ecsInterface.AddComponent<TextComponent>(newEntity, textComponent);
                // std::cout << "ADDED TEXT COMPONENT\n";
            }

            // Check and add PlayerComponent
            if (components.HasMember("PlayerComponent") && components["PlayerComponent"].IsObject())
            {
                const rapidjson::Value& playerComp = components["PlayerComponent"];
                PlayerComponent playerComponent;

                // Read "CurrentText" field
                if (playerComp.HasMember("CurrentText") && playerComp["CurrentText"].IsString())
                {
                    playerComponent.CurrentText = playerComp["CurrentText"].GetString();
                }

                if (playerComp.HasMember("type")) {
                    std::string typeStr = playerComp["type"].GetString();
                    if (typeStr == "Player") {
                        playerComponent.type = Player;
                    }
                    else if (typeStr == "TextBox") {
                        playerComponent.type = TextBox;
                    }
                }
                if (playerComp.HasMember("health") && playerComp["health"].IsFloat()) {
                    playerComponent.health = playerComp["health"].GetFloat();
                }
                // Add PlayerComponent to entity
                ecsInterface.AddComponent<PlayerComponent>(newEntity, playerComponent);
                // std::cout << "ADDED PLAYER COMPONENT\n";
            }
            //Add Spawner Component
            if (components.HasMember("SpawnerComponent") && components["SpawnerComponent"].IsObject())
            {
                const rapidjson::Value& spawnerComp = components["SpawnerComponent"];
                SpawnerComponent spawnerComponent;

                if (spawnerComp.HasMember("accumulatedTime") && spawnerComp["accumulatedTime"].IsFloat()) {
                    spawnerComponent.accumulatedTime = spawnerComp["accumulatedTime"].GetFloat();
                }

                if (spawnerComp.HasMember("spawnInterval") && spawnerComp["spawnInterval"].IsFloat()) {
                    spawnerComponent.accumulatedTime = spawnerComp["spawnInterval"].GetFloat();
                }
                // Add SpawnerComponent to entity
                ecsInterface.AddComponent<SpawnerComponent>(newEntity, spawnerComponent);
                // std::cout << "ADDED SPAWN COMPONENT\n";
            }

            //Check if it has movement component
            if (components.HasMember("MovementComponent") && components["MovementComponent"].IsObject()) {

                const rapidjson::Value& movement = components["MovementComponent"];

                MovementComponent movementComponent;

                if (movement.HasMember("x")) movementComponent.velocity.x = movement["x"].GetFloat();
                if (movement.HasMember("y")) movementComponent.velocity.y = movement["y"].GetFloat();
                if (movement.HasMember("baseX")) movementComponent.baseVelocity.x = movement["baseX"].GetFloat();
                if (movement.HasMember("baseY")) movementComponent.baseVelocity.y = movement["baseY"].GetFloat();
                

                ecsInterface.AddComponent<MovementComponent>(newEntity, movementComponent);

                //std::cout << "ADDED MOVEMENT COMPONENT\n";
            }

            //Check if it has collision component
            if (components.HasMember("CollisionComponent") && components["CollisionComponent"].IsObject()) {
                const rapidjson::Value& collision = components["CollisionComponent"];
                CollisionComponent collisionComponent;
                if (collision.HasMember("type")) {
                    std::string typeStr = collision["type"].GetString();
                    if (typeStr == "Player") {
                        collisionComponent.type = Player;
                    }
                    else if (typeStr == "Enemy") {

                        collisionComponent.type = Enemy;
                    }
                    else if (typeStr == "CollidableObject") {
                        collisionComponent.type = CollidableObject;
                    }
                }
                if (collision.HasMember("collided")) collisionComponent.collided = collision["collided"].GetBool();
                if (collision.HasMember("radius")) collisionComponent.radius = collision["radius"].GetFloat();
                if (collision.HasMember("collisionScaleX")) collisionComponent.scale.x = collision["collisionScaleX"].GetFloat();
                if (collision.HasMember("collisionScaleY")) collisionComponent.scale.y = collision["collisionScaleY"].GetFloat();
                ecsInterface.AddComponent<CollisionComponent>(newEntity, collisionComponent);
                //std::cout << "ADDED COLLISION COMPONENT\n";
            }

            // Check and add Enemy Component
            if (components.HasMember("EnemyComponent") && components["EnemyComponent"].IsObject()) {
                const rapidjson::Value& enemy = components["EnemyComponent"];
                EnemyComponent enemyComponent;

                // Load and set the enemy type
                if (enemy.HasMember("type") && enemy["type"].IsString()) {
                    std::string typeStr = enemy["type"].GetString();
                    if (typeStr == "Minion") {
                        enemyComponent.type = Minion;
                    }
                    else if (typeStr == "Boss") {
                        enemyComponent.type = Boss;
                    }
                    else if (typeStr == "MC") {
                        enemyComponent.type = MC;
                    }
                    else if (typeStr == "Poison") {
                        enemyComponent.type = Poison;
                    }
                    else if (typeStr == "Spawner") {
                        enemyComponent.type = Spawner;
                    }
                    else if (typeStr == "Smoke") {
                        enemyComponent.type = Smoke;
                    }
                }

                // Load health and predicted health value
                if (enemy.HasMember("health") && enemy["health"].IsFloat()) {
                    enemyComponent.health = enemy["health"].GetFloat();
                    enemyComponent.predictedHealth = enemy["health"].GetFloat();
                    enemyComponent.Maxhealth = enemy["health"].GetFloat();
                }
                
               
                // Load the UpdateFunctionName
                if (enemy.HasMember("UpdateFunctionName") && enemy["UpdateFunctionName"].IsString()) {
                    enemyComponent.UpdateFunctionName = enemy["UpdateFunctionName"].GetString();

                    // Retrieve the behavior function from LogicManager
                    BehaviorFunction behaviorFunction = GlobalLogicManager.GetFunction(enemyComponent.UpdateFunctionName);

                    if (behaviorFunction) {
                        enemyComponent.behavior = behaviorFunction;
                        std::cout << "Assigned behavior function '" << enemyComponent.UpdateFunctionName
                            << "' to entity " << newEntity << std::endl;
                    }
                    else {
                        std::cerr << "Warning: Behavior function '" << enemyComponent.UpdateFunctionName
                            << "' not found for entity " << newEntity << std::endl;
                    }
                }
                // Load spawn status
                if (enemy.HasMember("spawned") && enemy["spawned"].IsBool()) {
                    enemyComponent.spawned = enemy["spawned"].GetBool();
                }
                // Load health value
                if (enemy.HasMember("spawnRate") && enemy["spawnRate"].IsFloat()) {
                    enemyComponent.spawnRate = enemy["spawnRate"].GetFloat();
                }
                if (enemy.HasMember("spawnTimer") && enemy["spawnTimer"].IsFloat()) {
                    enemyComponent.spawnTimer = enemy["spawnTimer"].GetFloat();
                }

                // Add EnemyComponent to the entity
                ecsInterface.AddComponent<EnemyComponent>(newEntity, enemyComponent);
                std::cout << "ADDED ENEMY COMPONENT to entity " << newEntity << std::endl;
            }

            // Check and add Animation Component
            if (components.HasMember("AnimationComponent") && components["AnimationComponent"].IsObject()) {
                const rapidjson::Value& animation = components["AnimationComponent"];
                AnimationComponent animationComponent;

       
                if (animation.HasMember("animationSpeed")) animationComponent.animationSpeed = animation["animationSpeed"].GetFloat();
                if (animation.HasMember("rows")) animationComponent.rows = animation["rows"].GetInt(); std::cout << animation["rows"].GetInt() << std::endl;
                if (animation.HasMember("cols")) animationComponent.cols = animation["cols"].GetInt();

                ecsInterface.AddComponent<AnimationComponent>(newEntity, animationComponent);
                //std::cout << "ADDED ANIMATION COMPONENT\n";
            }

            //Check if it has Bullet component
            if (components.HasMember("BulletComponent") && components["BulletComponent"].IsObject()) {

                const rapidjson::Value& bullet = components["BulletComponent"];

                BulletComponent bulletComponent;

                if (bullet.HasMember("targetId")) bulletComponent.targetId = bullet["targetId"].GetUint();

                ecsInterface.AddComponent<BulletComponent>(newEntity, bulletComponent);

                //std::cout << "ADDED MOVEMENT COMPONENT\n";
            }
            // Check and add ButtonComponent
            if (components.HasMember("ButtonComponent") && components["ButtonComponent"].IsObject()) {
                const rapidjson::Value& buttonComp = components["ButtonComponent"];
                ButtonComponent buttonComponent;

                // Parse label
                if (buttonComp.HasMember("label") && buttonComp["label"].IsString()) {
                    buttonComponent.label = buttonComp["label"].GetString();
                }
                else {
                    std::cerr << "Warning: Missing or invalid 'label' for ButtonComponent in entity " << newEntity << std::endl;
                    buttonComponent.label = "DefaultLabel"; // Default value
                }

                // Parse texture IDs
                if (buttonComp.HasMember("idleTextureID") && buttonComp["idleTextureID"].IsString()) {
                    buttonComponent.idleTextureID = buttonComp["idleTextureID"].GetString();
                }
                else {
                    std::cerr << "Warning: Missing or invalid 'idleTextureID' for ButtonComponent in entity " << newEntity << std::endl;
                }

                if (buttonComp.HasMember("hoverTextureID") && buttonComp["hoverTextureID"].IsString()) {
                    buttonComponent.hoverTextureID = buttonComp["hoverTextureID"].GetString();
                }
                else {
                    std::cerr << "Warning: Missing or invalid 'hoverTextureID' for ButtonComponent in entity " << newEntity << std::endl;
                }

                if (buttonComp.HasMember("pressedTextureID") && buttonComp["pressedTextureID"].IsString()) {
                    buttonComponent.pressedTextureID = buttonComp["pressedTextureID"].GetString();
                }
                else {
                    std::cerr << "Warning: Missing or invalid 'pressedTextureID' for ButtonComponent in entity " << newEntity << std::endl;
                }

                // Parse pressCooldown
                if (buttonComp.HasMember("pressCooldown") && buttonComp["pressCooldown"].IsFloat()) {
                    buttonComponent.pressCooldown = buttonComp["pressCooldown"].GetFloat();
                }
                else {
                    std::cerr << "Warning: Missing or invalid 'pressCooldown' for ButtonComponent in entity " << newEntity << std::endl;
                    buttonComponent.pressCooldown = 0.2f; // Default value
                }

                // Parse UpdateFunctionName and map to onClick
                if (buttonComp.HasMember("onClick") && buttonComp["onClick"].IsString()) {
                    buttonComponent.UpdateFunctionName = buttonComp["onClick"].GetString();
                    auto buttonFunction = GlobalLogicManager.GetButtonFunction(buttonComponent.UpdateFunctionName);

                    if (buttonFunction) {
                        buttonComponent.onClick = [newEntity, buttonFunction]() {
                            buttonFunction(newEntity);
                            };
                    }
                    else {
                        std::cerr << "Warning: Button click event '" << buttonComponent.UpdateFunctionName
                            << "' not found for entity " << newEntity << std::endl;
                    }
                }
                else {
                    std::cerr << "Warning: Missing or invalid 'onClick' for ButtonComponent in entity " << newEntity << std::endl;
                }

                // Add the ButtonComponent to the entity
                ecsInterface.AddComponent<ButtonComponent>(newEntity, buttonComponent);
            }

            // Check and add TimelineComponent
            // Deserialize TimelineComponent
            if (components.HasMember("TimelineComponent") && components["TimelineComponent"].IsObject()) 
            {
                const rapidjson::Value& timelineComp = components["TimelineComponent"];
                TimelineComponent timelineComponent;

                // Parse InternalTimer
                if (timelineComp.HasMember("InternalTimer") && timelineComp["InternalTimer"].IsFloat()) {
                    timelineComponent.InternalTimer = timelineComp["InternalTimer"].GetFloat();
                }
                else {
                    std::cerr << "Warning: Missing or invalid 'InternalTimer' for TimelineComponent in entity " << newEntity << std::endl;
                    timelineComponent.InternalTimer = 0.0f; // Default value
                }

                // Parse TransitionDuration
                if (timelineComp.HasMember("TransitionDuration") && timelineComp["TransitionDuration"].IsFloat()) {
                    timelineComponent.TransitionDuration = timelineComp["TransitionDuration"].GetFloat();
                }
                else {
                    std::cerr << "Warning: Missing or invalid 'TransitionDuration' for TimelineComponent in entity " << newEntity << std::endl;
                    timelineComponent.TransitionDuration = 1.0f; // Default value
                }

                // Parse TransitionInDelay
                if (timelineComp.HasMember("TransitionInDelay") && timelineComp["TransitionInDelay"].IsFloat()) {
                    timelineComponent.TransitionInDelay = timelineComp["TransitionInDelay"].GetFloat();
                }
                else {
                    std::cerr << "Warning: Missing or invalid 'TransitionInDelay' for TimelineComponent in entity " << newEntity << std::endl;
                    timelineComponent.TransitionInDelay = 1.0f; // Default value
                }

                // Parse TransitionOutDelay
                if (timelineComp.HasMember("TransitionOutDelay") && timelineComp["TransitionOutDelay"].IsFloat()) {
                    timelineComponent.TransitionOutDelay = timelineComp["TransitionOutDelay"].GetFloat();
                }
                else {
                    std::cerr << "Warning: Missing or invalid 'TransitionOutDelay' for TimelineComponent in entity " << newEntity << std::endl;
                    timelineComponent.TransitionOutDelay = 1.0f; // Default value
                }

                // Parse TransitionInFunctionName and map to TransitionIn
                if (timelineComp.HasMember("TransitionInFunctionName") && timelineComp["TransitionInFunctionName"].IsString()) {
                    timelineComponent.TransitionInFunctionName = timelineComp["TransitionInFunctionName"].GetString();
                    auto transitionInFunction = GlobalLogicManager.GetTimelineFunction(timelineComponent.TransitionInFunctionName);

                    if (transitionInFunction) {
                        timelineComponent.TransitionIn = [newEntity, transitionInFunction](Framework::Entity e, float progress) {
                            (void)e;
                            transitionInFunction(newEntity, progress);
                            };
                    }
                    else {
                        std::cerr << "Warning: Transition In function '" << timelineComponent.TransitionInFunctionName
                            << "' not found for entity " << newEntity << std::endl;
                    }
                }
                else {
                    std::cerr << "Warning: Missing or invalid 'TransitionInFunctionName' for TimelineComponent in entity " << newEntity << std::endl;
                }

                // Parse TransitionOutFunctionName and map to TransitionOut
                if (timelineComp.HasMember("TransitionOutFunctionName") && timelineComp["TransitionOutFunctionName"].IsString()) {
                    timelineComponent.TransitionOutFunctionName = timelineComp["TransitionOutFunctionName"].GetString();
                    auto transitionOutFunction = GlobalLogicManager.GetTimelineFunction(timelineComponent.TransitionOutFunctionName);

                    if (transitionOutFunction) {
                        timelineComponent.TransitionOut = [newEntity, transitionOutFunction](Framework::Entity e, float progress) {
                            (void)e;
                            transitionOutFunction(newEntity, progress);
                            };
                    }
                    else {
                        std::cerr << "Warning: Transition Out function '" << timelineComponent.TransitionOutFunctionName
                            << "' not found for entity " << newEntity << std::endl;
                    }
                }
                else {
                    std::cerr << "Warning: Missing or invalid 'TransitionOutFunctionName' for TimelineComponent in entity " << newEntity << std::endl;
                }

                // Parse Active
                if (timelineComp.HasMember("Active") && timelineComp["Active"].IsBool()) {
                    timelineComponent.Active = timelineComp["Active"].GetBool();
                }
                else {
                    std::cerr << "Warning: Missing or invalid 'Active' for TimelineComponent in entity " << newEntity << std::endl;
                    timelineComponent.Active = false; // Default value
                }

                // Parse IsTransitioningIn
                if (timelineComp.HasMember("IsTransitioningIn") && timelineComp["IsTransitioningIn"].IsBool()) {
                    timelineComponent.IsTransitioningIn = timelineComp["IsTransitioningIn"].GetBool();
                }
                else {
                    std::cerr << "Warning: Missing or invalid 'IsTransitioningIn' for TimelineComponent in entity " << newEntity << std::endl;
                    timelineComponent.IsTransitioningIn = true; // Default value
                }

                // Parse TimelineTag
                if (timelineComp.HasMember("TimelineTag") && timelineComp["TimelineTag"].IsString()) {
                    timelineComponent.TimelineTag = timelineComp["TimelineTag"].GetString();
                }
                else {
                    std::cerr << "Warning: Missing or invalid 'TimelineTag' for TimelineComponent in entity " << newEntity << std::endl;
                    timelineComponent.TimelineTag = "DefaultTag"; // Default value
                }

                // Parse startPosition
                if (timelineComp.HasMember("startPosition") && (timelineComp["startPosition"].IsFloat() || timelineComp["startPosition"].IsInt())) {
                    timelineComponent.startPosition = timelineComp["startPosition"].GetFloat();
                }
                else {
                    std::cerr << "Warning: Missing or invalid 'startPosition' for TimelineComponent in entity " << newEntity << std::endl;
                    timelineComponent.startPosition = 0.f; // Default value
                }

                // Parse endPosition
                if (timelineComp.HasMember("endPosition") && (timelineComp["endPosition"].IsFloat()|| timelineComp["endPosition"].IsInt())) {
                    timelineComponent.endPosition = timelineComp["endPosition"].GetFloat();
                }
                else {
                    std::cerr << "Warning: Missing or invalid 'endPosition' for TimelineComponent in entity " << newEntity << std::endl;
                    timelineComponent.endPosition = 0.f; // Default value
                }

                // Add the TimelineComponent to the entity
                ecsInterface.AddComponent<TimelineComponent>(newEntity, timelineComponent);

                // Initialize the TimelineComponent's transition functions
                GlobalLogicManager.InitializeTimeline(newEntity);
            }

            // Check if it has ParticleComponent
            if (components.HasMember("ParticleComponent") && components["ParticleComponent"].IsObject()) 
            {
                const rapidjson::Value& particle = components["ParticleComponent"];

                ParticleComponent particleComponent;

                if (particle.HasMember("positionX")) particleComponent.position.x = particle["positionX"].GetFloat();
                if (particle.HasMember("positionY")) particleComponent.position.y = particle["positionY"].GetFloat();
                if (particle.HasMember("velocityX")) particleComponent.velocity.x = particle["velocityX"].GetFloat();
                if (particle.HasMember("velocityY")) particleComponent.velocity.y = particle["velocityY"].GetFloat();
                if (particle.HasMember("colorR")) particleComponent.color.r = particle["colorR"].GetFloat();
                if (particle.HasMember("colorG")) particleComponent.color.g = particle["colorG"].GetFloat();
                if (particle.HasMember("colorB")) particleComponent.color.b = particle["colorB"].GetFloat();
                if (particle.HasMember("size")) particleComponent.size = particle["size"].GetFloat();
                if (particle.HasMember("life")) particleComponent.life = particle["life"].GetFloat();
                if (particle.HasMember("active")) particleComponent.active = particle["active"].GetBool();
                if (particle.HasMember("emissionRate")) particleComponent.emissionRate = particle["emissionRate"].GetFloat();
                
                // Deserialize texture name
                if (particle.HasMember("textureName") && particle["textureName"].IsString()) {
                    particleComponent.textureName = particle["textureName"].GetString();
                }

                // Read EmissionShape from string
                if (particle.HasMember("shape") && particle["shape"].IsString()) {
                    std::string shapeStr = particle["shape"].GetString();
                    if (shapeStr == "CIRCLE") particleComponent.shape = EmissionShape::CIRCLE;
                    else if (shapeStr == "BOX") particleComponent.shape = EmissionShape::BOX;
                    else if (shapeStr == "ELLIPSE") particleComponent.shape = EmissionShape::ELLIPSE;
                    else if (shapeStr == "LINE") particleComponent.shape = EmissionShape::LINE;
                    else if (shapeStr == "SPIRAL") particleComponent.shape = EmissionShape::SPIRAL;
                    else if (shapeStr == "RADIAL") particleComponent.shape = EmissionShape::RADIAL;
                    else if (shapeStr == "RANDOM") particleComponent.shape = EmissionShape::RANDOM;
                    else if (shapeStr == "WAVE") particleComponent.shape = EmissionShape::WAVE;
                    else if (shapeStr == "CONE") particleComponent.shape = EmissionShape::CONE;
                    else if (shapeStr == "EXPLOSION") particleComponent.shape = EmissionShape::EXPLOSION;
                }

                // Load shape-specific data
                if (particle.HasMember("radius")) particleComponent.radius = particle["radius"].GetFloat();
                if (particle.HasMember("boxSizeX")) particleComponent.boxSize.x = particle["boxSizeX"].GetFloat();
                if (particle.HasMember("boxSizeY")) particleComponent.boxSize.y = particle["boxSizeY"].GetFloat();
                if (particle.HasMember("spiralTurns")) particleComponent.spiralTurns = particle["spiralTurns"].GetFloat();
                if (particle.HasMember("coneAngle")) particleComponent.coneAngle = particle["coneAngle"].GetFloat();

                ecsInterface.AddComponent<ParticleComponent>(newEntity, particleComponent);

                std::cout << "Deserialized ParticleComponent for entity: " << newEntity << std::endl;
            }

            // check if it has and add bar component
            if (components.HasMember("UIBarComponent") && components["UIBarComponent"].IsObject())
            {
                const rapidjson::Value& bar = components["UIBarComponent"];
                UIBarComponent barComponent;

                // Texture IDs
                if (bar.HasMember("backingTextureID") && bar["backingTextureID"].IsString())
                    barComponent.backingTextureID = bar["backingTextureID"].GetString();

                if (bar.HasMember("fillTextureID") && bar["fillTextureID"].IsString())
                    barComponent.fillTextureID = bar["fillTextureID"].GetString();

                // Fill %
                if (bar.HasMember("fillPercentage") && bar["fillPercentage"].IsNumber())
                    barComponent.FillPercentage = bar["fillPercentage"].GetFloat();

                // Offset
                if (bar.HasMember("offsetX")) barComponent.offset.x = bar["offsetX"].GetFloat();
                if (bar.HasMember("offsetY")) barComponent.offset.y = bar["offsetY"].GetFloat();

                // Scale
                if (bar.HasMember("scaleX")) barComponent.scale.x = bar["scaleX"].GetFloat();
                if (bar.HasMember("scaleY")) barComponent.scale.y = bar["scaleY"].GetFloat();

                // Fill Offset
                if (bar.HasMember("fillOffsetX")) barComponent.fillOffset.x = bar["fillOffsetX"].GetFloat();
                if (bar.HasMember("fillOffsetY")) barComponent.fillOffset.y = bar["fillOffsetY"].GetFloat();

                // Fill Size
                if (bar.HasMember("fillSizeX")) barComponent.fillSize.x = bar["fillSizeX"].GetFloat();
                if (bar.HasMember("fillSizeY")) barComponent.fillSize.y = bar["fillSizeY"].GetFloat();

                // Fill color (vec3 array)
                if (bar.HasMember("fillColor") && bar["fillColor"].IsArray()) {
                    const auto& color = bar["fillColor"].GetArray();
                    if (color.Size() == 3) {
                        barComponent.fillColor = glm::vec3(
                            color[0].GetFloat(),
                            color[1].GetFloat(),
                            color[2].GetFloat()
                        );
                    }
                }

                // Fill alpha
                if (bar.HasMember("fillAlpha") && bar["fillAlpha"].IsNumber()) {
                    barComponent.fillAlpha = bar["fillAlpha"].GetFloat();
                }

                // Background color (vec3 array)
                if (bar.HasMember("bgColor") && bar["bgColor"].IsArray()) {
                    const auto& bg = bar["bgColor"].GetArray();
                    if (bg.Size() == 3) {
                        barComponent.bgColor = glm::vec3(
                            bg[0].GetFloat(),
                            bg[1].GetFloat(),
                            bg[2].GetFloat()
                        );
                    }
                }

                // Background alpha
                if (bar.HasMember("bgAlpha") && bar["bgAlpha"].IsNumber()) {
                    barComponent.bgAlpha = bar["bgAlpha"].GetFloat();
                }

                ecsInterface.AddComponent<UIBarComponent>(newEntity, barComponent);
                std::cout << "Added UIBarComponent to entity " << newEntity << "\n";
            }

        }
    }
}

void EntityAsset::SerializeEntities(const std::string& filename)
{
    std::cout << "Serializing to: " << filename << std::endl;

    // Create a JSON document
    rapidjson::Document document;
    document.SetObject();

    // Create the "entities" array
    rapidjson::Value entities(rapidjson::kArrayType);

    auto& entityAsset = ecsInterface.GetEntities(); // Get all entities
    std::cout << "Found " << entityAsset.size() << " entities." << std::endl;

    // Iterate through all entities
    for (const auto& entity : entityAsset)
    {
        std::cout << "Serializing entity: " << entity << std::endl;

        rapidjson::Value entityObj(rapidjson::kObjectType);

        // Add type of the entity (use GetEntityName if applicable)
        std::string entityName = ecsInterface.GetEntityName(entity);
        std::cout << "Entity name: " << entityName << std::endl;

        // Use "type" instead of "name" for entity type
        entityObj.AddMember("type", rapidjson::Value(entityName.c_str(), document.GetAllocator()), document.GetAllocator());

        // Create a "components" object for the entity
        rapidjson::Value components(rapidjson::kObjectType);

        // Serialize TransformComponent
        if (ecsInterface.HasComponent<TransformComponent>(entity)) {
            const auto& transform = ecsInterface.GetComponent<TransformComponent>(entity);
            std::cout << "Serializing TransformComponent for entity: " << entity << std::endl;

            rapidjson::Value transformComp(rapidjson::kObjectType);
            transformComp.AddMember("x", transform.position.x, document.GetAllocator());
            transformComp.AddMember("y", transform.position.y, document.GetAllocator());
            transformComp.AddMember("scaleX", transform.scale.x, document.GetAllocator());
            transformComp.AddMember("scaleY", transform.scale.y, document.GetAllocator());
            transformComp.AddMember("rotation", transform.rotation, document.GetAllocator());

            // **NEW: Add Tag to Serialization**
            rapidjson::Value tagValue;
            tagValue.SetString(transform.tag.c_str(), static_cast<rapidjson::SizeType>(transform.tag.length()), document.GetAllocator());
            transformComp.AddMember("tag", tagValue, document.GetAllocator());
            
            components.AddMember("TransformComponent", transformComp, document.GetAllocator());
        }


        // Serialize RenderComponent
        if (ecsInterface.HasComponent<RenderComponent>(entity)) {
            const auto& render = ecsInterface.GetComponent<RenderComponent>(entity);
            std::cout << "Serializing RenderComponent for entity: " << entity << std::endl;

            rapidjson::Value renderComp(rapidjson::kObjectType);
            renderComp.AddMember("textureID", rapidjson::Value(render.textureID.c_str(), document.GetAllocator()), document.GetAllocator());

            // Serialize color as an array
            rapidjson::Value colorArray(rapidjson::kArrayType);
            colorArray.PushBack(render.color.r, document.GetAllocator());
            colorArray.PushBack(render.color.g, document.GetAllocator());
            colorArray.PushBack(render.color.b, document.GetAllocator());
            renderComp.AddMember("color", colorArray, document.GetAllocator());

            renderComp.AddMember("alpha", render.alpha, document.GetAllocator());

            // Serialize renderType as a string
            std::string renderTypeStr;
            switch (render.renderType) {
            case RenderType::Sprite: renderTypeStr = "Sprite"; break;
            case RenderType::Particle: renderTypeStr = "Particle"; break;
            case RenderType::Text: renderTypeStr = "Text"; break;
            case RenderType::PauseUI: renderTypeStr = "PauseUI"; break;
            }
            renderComp.AddMember("renderType", rapidjson::Value(renderTypeStr.c_str(), document.GetAllocator()), document.GetAllocator());

            components.AddMember("RenderComponent", renderComp, document.GetAllocator());
        }

        // Serialize TextComponent
        if (ecsInterface.HasComponent<TextComponent>(entity)) {
            const auto& text = ecsInterface.GetComponent<TextComponent>(entity);
            std::cout << "Serializing TextComponent for entity: " << entity << std::endl;

            rapidjson::Value textComp(rapidjson::kObjectType);

            textComp.AddMember("text", rapidjson::Value(text.text.c_str(), document.GetAllocator()), document.GetAllocator());
            textComp.AddMember("fontSize", text.fontSize, document.GetAllocator());

            // Serialize color as an array
            rapidjson::Value colorArray(rapidjson::kArrayType);
            colorArray.PushBack(text.color.r, document.GetAllocator());
            colorArray.PushBack(text.color.g, document.GetAllocator());
            colorArray.PushBack(text.color.b, document.GetAllocator());
            textComp.AddMember("color", colorArray, document.GetAllocator());

            textComp.AddMember("fontName", rapidjson::Value(text.fontName.c_str(), document.GetAllocator()), document.GetAllocator());

            // Serialize offset as an array
            rapidjson::Value offsetArray(rapidjson::kArrayType);
            offsetArray.PushBack(text.offset.x, document.GetAllocator());
            offsetArray.PushBack(text.offset.y, document.GetAllocator());
            textComp.AddMember("offset", offsetArray, document.GetAllocator());

            components.AddMember("TextComponent", textComp, document.GetAllocator());
        }

        // Serialize LayerComponent
        if (ecsInterface.HasComponent<LayerComponent>(entity)) {
            const auto& layer = ecsInterface.GetComponent<LayerComponent>(entity);
            std::cout << "Serializing LayerComponent for entity: " << entity << std::endl;

            rapidjson::Value layerComp(rapidjson::kObjectType);

            layerComp.AddMember("LayerID", static_cast<int>(layer.layerID), document.GetAllocator());
            layerComp.AddMember("SortID", layer.sortID, document.GetAllocator());

            components.AddMember("LayerComponent", layerComp, document.GetAllocator());
        }

        // Serialize MovementComponent
        if (ecsInterface.HasComponent<MovementComponent>(entity)) {
            const auto& movement = ecsInterface.GetComponent<MovementComponent>(entity);
            std::cout << "Serializing MovementComponent for entity: " << entity << std::endl;

            rapidjson::Value movementComp(rapidjson::kObjectType);
            movementComp.AddMember("x", movement.velocity.x, document.GetAllocator());
            movementComp.AddMember("y", movement.velocity.y, document.GetAllocator());
            movementComp.AddMember("baseX", movement.baseVelocity.x, document.GetAllocator());
            movementComp.AddMember("baseY", movement.baseVelocity.y, document.GetAllocator());
            components.AddMember("MovementComponent", movementComp, document.GetAllocator());
        }

        // Serialize CollisionComponent
        if (ecsInterface.HasComponent<CollisionComponent>(entity)) {
            const auto& collision = ecsInterface.GetComponent<CollisionComponent>(entity);
            std::cout << "Serializing CollisionComponent for entity: " << entity << std::endl;

            rapidjson::Value collisionComp(rapidjson::kObjectType);
            collisionComp.AddMember("type", rapidjson::Value(ObjectTypeToString(collision.type).c_str(), document.GetAllocator()), document.GetAllocator());
            collisionComp.AddMember("collided", collision.collided, document.GetAllocator());
            collisionComp.AddMember("collisionScaleX", collision.scale.x, document.GetAllocator());
            collisionComp.AddMember("collisionScaleY", collision.scale.y, document.GetAllocator());
            collisionComp.AddMember("radius", collision.radius, document.GetAllocator());
            components.AddMember("CollisionComponent", collisionComp, document.GetAllocator());
        }

        // Serialize EnemyComponent
        if (ecsInterface.HasComponent<EnemyComponent>(entity)) {
            const auto& enemy = ecsInterface.GetComponent<EnemyComponent>(entity);
            std::cout << "Serializing EnemyComponent for entity: " << entity << std::endl;

            rapidjson::Value enemyComp(rapidjson::kObjectType);
            enemyComp.AddMember("type", rapidjson::Value(EnemyTypeToString(enemy.type).c_str(), document.GetAllocator()), document.GetAllocator());
            enemyComp.AddMember("health", enemy.health, document.GetAllocator());
            enemyComp.AddMember("UpdateFunctionName", rapidjson::Value(enemy.UpdateFunctionName.c_str(), document.GetAllocator()), document.GetAllocator());
            enemyComp.AddMember("spawned", enemy.spawned, document.GetAllocator());
            enemyComp.AddMember("spawnRate", enemy.spawnRate, document.GetAllocator());
            enemyComp.AddMember("spawnTimer", enemy.spawnTimer, document.GetAllocator());
            components.AddMember("EnemyComponent", enemyComp, document.GetAllocator());
        }
        // Spawner Component
        if (ecsInterface.HasComponent<SpawnerComponent>(entity)) {
            const auto& spawner = ecsInterface.GetComponent<SpawnerComponent>(entity);
            std::cout << "Serializing SpawnerComponent for entity: " << entity << std::endl;

            rapidjson::Value spawnerComp(rapidjson::kObjectType);
            spawnerComp.AddMember("accumulatedTime", spawner.accumulatedTime, document.GetAllocator());
            spawnerComp.AddMember("spawnInterval", spawner.spawnInterval, document.GetAllocator());
            components.AddMember("SpawnerComponent", spawnerComp, document.GetAllocator());
        }

        // Serialize AnimationComponent
        if (ecsInterface.HasComponent<AnimationComponent>(entity)) {
            const auto& animation = ecsInterface.GetComponent<AnimationComponent>(entity);
            std::cout << "Serializing AnimationComponent for entity: " << entity << std::endl;

            rapidjson::Value animationComp(rapidjson::kObjectType);
            animationComp.AddMember("animationSpeed", animation.animationSpeed, document.GetAllocator());
            animationComp.AddMember("rows", animation.rows, document.GetAllocator());
            animationComp.AddMember("cols", animation.cols, document.GetAllocator());
            components.AddMember("AnimationComponent", animationComp, document.GetAllocator());
        }

        // Serialize BulletComponent
        if (ecsInterface.HasComponent<BulletComponent>(entity)) {
            const auto& bullet = ecsInterface.GetComponent<BulletComponent>(entity);
            std::cout << "Serializing BulletComponent for entity: " << entity << std::endl;

            rapidjson::Value bulletComp(rapidjson::kObjectType);
            bulletComp.AddMember("targetId", bullet.targetId, document.GetAllocator());
            components.AddMember("BulletComponent", bulletComp, document.GetAllocator());
        }

        // Serialize the ButtonComponent
        if (ecsInterface.HasComponent<ButtonComponent>(entity)) {
            const auto& button = ecsInterface.GetComponent<ButtonComponent>(entity);
            std::cout << "Serializing ButtonComponent for entity: " << entity << std::endl;

            rapidjson::Value buttonComp(rapidjson::kObjectType);

            // Serialize basic properties
            buttonComp.AddMember("label", rapidjson::Value(button.label.c_str(), document.GetAllocator()), document.GetAllocator());
            buttonComp.AddMember("idleTextureID", rapidjson::Value(button.idleTextureID.c_str(), document.GetAllocator()), document.GetAllocator());
            buttonComp.AddMember("hoverTextureID", rapidjson::Value(button.hoverTextureID.c_str(), document.GetAllocator()), document.GetAllocator());
            buttonComp.AddMember("pressedTextureID", rapidjson::Value(button.pressedTextureID.c_str(), document.GetAllocator()), document.GetAllocator());

            // Serialize the update function name
            buttonComp.AddMember("UpdateFunctionName", rapidjson::Value(button.UpdateFunctionName.c_str(), document.GetAllocator()), document.GetAllocator());
            buttonComp.AddMember("onClick", rapidjson::Value(button.UpdateFunctionName.c_str(), document.GetAllocator()), document.GetAllocator());

            // Serialize additional properties
            buttonComp.AddMember("PressedAudio", rapidjson::Value(button.PressedAudio.c_str(), document.GetAllocator()), document.GetAllocator());
            buttonComp.AddMember("HoverAudio", rapidjson::Value(button.HoverAudio.c_str(), document.GetAllocator()), document.GetAllocator());
            buttonComp.AddMember("FirstHover", button.FirstHover, document.GetAllocator());
            buttonComp.AddMember("pressCooldown", button.pressCooldown, document.GetAllocator());
            buttonComp.AddMember("pressTimeRemaining", button.pressTimeRemaining, document.GetAllocator());

            // Serialize button state
            const char* buttonStateStr = nullptr;
            switch (button.state) {
            case ButtonState::Idle: buttonStateStr = "Idle"; break;
            case ButtonState::Hover: buttonStateStr = "Hover"; break;
            case ButtonState::Pressed: buttonStateStr = "Pressed"; break;
            }
            if (buttonStateStr) {
                buttonComp.AddMember("state", rapidjson::Value(buttonStateStr, document.GetAllocator()), document.GetAllocator());
            }

            // Add the ButtonComponent to the components object
            components.AddMember("ButtonComponent", buttonComp, document.GetAllocator());
        }

        // Serialize the TimelineComponent
        if (ecsInterface.HasComponent<TimelineComponent>(entity)) {
            const auto& timeline = ecsInterface.GetComponent<TimelineComponent>(entity);
            std::cout << "Serializing TimelineComponent for entity: " << entity << std::endl;
            rapidjson::Value timelineComp(rapidjson::kObjectType);

            // Serialize basic properties
            timelineComp.AddMember("InternalTimer", timeline.InternalTimer, document.GetAllocator());
            timelineComp.AddMember("TransitionDuration", timeline.TransitionDuration, document.GetAllocator());
            timelineComp.AddMember("TransitionInDelay", timeline.TransitionInDelay, document.GetAllocator());
            timelineComp.AddMember("TransitionOutDelay", timeline.TransitionOutDelay, document.GetAllocator());
            timelineComp.AddMember("TransitionInFunctionName", rapidjson::Value(timeline.TransitionInFunctionName.c_str(), document.GetAllocator()), document.GetAllocator());
            timelineComp.AddMember("TransitionOutFunctionName", rapidjson::Value(timeline.TransitionOutFunctionName.c_str(), document.GetAllocator()), document.GetAllocator());

            // Serialize active and transition state
            timelineComp.AddMember("Active", timeline.Active, document.GetAllocator());
            timelineComp.AddMember("IsTransitioningIn", timeline.IsTransitioningIn, document.GetAllocator());

            // Serialize additional properties
            timelineComp.AddMember("TimelineTag", rapidjson::Value(timeline.TimelineTag.c_str(), document.GetAllocator()), document.GetAllocator());
            timelineComp.AddMember("startPosition", timeline.startPosition, document.GetAllocator());
            timelineComp.AddMember("endPosition", timeline.endPosition, document.GetAllocator());

            // Add the TimelineComponent to the components object
            components.AddMember("TimelineComponent", timelineComp, document.GetAllocator());
        }


        // Serialize PlayerComponent
        if (ecsInterface.HasComponent<PlayerComponent>(entity)) {
            const auto& player = ecsInterface.GetComponent<PlayerComponent>(entity);
            std::cout << "Serializing PlayerComponent for entity: " << entity << std::endl;

            // Create a JSON object for PlayerComponent
            rapidjson::Value playerComp(rapidjson::kObjectType);

            // Add the "CurrentText" field to the JSON object
            playerComp.AddMember("CurrentText", rapidjson::Value(player.CurrentText.c_str(), document.GetAllocator()), document.GetAllocator());
            playerComp.AddMember("type", rapidjson::Value(ObjectTypeToString(player.type).c_str(), document.GetAllocator()), document.GetAllocator());
            playerComp.AddMember("health", player.health, document.GetAllocator());

            // Add the PlayerComponent object to the components JSON object
            components.AddMember("PlayerComponent", playerComp, document.GetAllocator());
        }

        // Serialize ParticleComponent
        if (ecsInterface.HasComponent<ParticleComponent>(entity)) {
            const auto& particle = ecsInterface.GetComponent<ParticleComponent>(entity);
            std::cout << "Serializing ParticleComponent for entity: " << entity << std::endl;

            rapidjson::Value particleComp(rapidjson::kObjectType);
            particleComp.AddMember("positionX", particle.position.x, document.GetAllocator());
            particleComp.AddMember("positionY", particle.position.y, document.GetAllocator());
            particleComp.AddMember("velocityX", particle.velocity.x, document.GetAllocator());
            particleComp.AddMember("velocityY", particle.velocity.y, document.GetAllocator());
            particleComp.AddMember("colorR", particle.color.r, document.GetAllocator());
            particleComp.AddMember("colorG", particle.color.g, document.GetAllocator());
            particleComp.AddMember("colorB", particle.color.b, document.GetAllocator());
            particleComp.AddMember("size", particle.size, document.GetAllocator());
            particleComp.AddMember("life", particle.life, document.GetAllocator());
            particleComp.AddMember("active", particle.active, document.GetAllocator());
            particleComp.AddMember("emissionRate", particle.emissionRate, document.GetAllocator());
            
            // Serialize texture name
            if (!particle.textureName.empty()) {
                rapidjson::Value textureNameValue;
                textureNameValue.SetString(particle.textureName.c_str(), document.GetAllocator());
                particleComp.AddMember("textureName", textureNameValue, document.GetAllocator());
            }

            // Store EmissionShape as a string
            rapidjson::Value shapeStr;
            std::string shapeName;
            switch (particle.shape) 
            {
                case EmissionShape::CIRCLE: shapeName = "CIRCLE"; break;
                case EmissionShape::BOX: shapeName = "BOX"; break;
                case EmissionShape::ELLIPSE: shapeName = "ELLIPSE"; break;
                case EmissionShape::LINE: shapeName = "LINE"; break;
                case EmissionShape::SPIRAL: shapeName = "SPIRAL"; break;
                case EmissionShape::RADIAL: shapeName = "RADIAL"; break;
                case EmissionShape::RANDOM: shapeName = "RANDOM"; break;
                case EmissionShape::WAVE: shapeName = "WAVE"; break;
                case EmissionShape::CONE: shapeName = "CONE"; break;
                case EmissionShape::EXPLOSION: shapeName = "EXPLOSION"; break;
            }
            shapeStr.SetString(shapeName.c_str(), document.GetAllocator());
            particleComp.AddMember("shape", shapeStr, document.GetAllocator());

            // Save shape-specific data
            particleComp.AddMember("radius", particle.radius, document.GetAllocator());
            particleComp.AddMember("boxSizeX", particle.boxSize.x, document.GetAllocator());
            particleComp.AddMember("boxSizeY", particle.boxSize.y, document.GetAllocator());
            particleComp.AddMember("spiralTurns", particle.spiralTurns, document.GetAllocator());
            particleComp.AddMember("coneAngle", particle.coneAngle, document.GetAllocator());

            components.AddMember("ParticleComponent", particleComp, document.GetAllocator());
        }

        // Serialize UIBarComponent
        if (ecsInterface.HasComponent<UIBarComponent>(entity)) {
            const auto& bar = ecsInterface.GetComponent<UIBarComponent>(entity);
            std::cout << "Serializing UIBarComponent for entity: " << entity << std::endl;

            rapidjson::Value barComp(rapidjson::kObjectType);

            // Texture IDs
            rapidjson::Value backingTex;
            backingTex.SetString(bar.backingTextureID.c_str(), static_cast<rapidjson::SizeType>(bar.backingTextureID.length()), document.GetAllocator());
            barComp.AddMember("backingTextureID", backingTex, document.GetAllocator());

            rapidjson::Value fillTex;
            fillTex.SetString(bar.fillTextureID.c_str(), static_cast<rapidjson::SizeType>(bar.fillTextureID.length()), document.GetAllocator());
            barComp.AddMember("fillTextureID", fillTex, document.GetAllocator());

            // Fill Percentage
            barComp.AddMember("fillPercentage", bar.FillPercentage, document.GetAllocator());

            // Offset
            barComp.AddMember("offsetX", bar.offset.x, document.GetAllocator());
            barComp.AddMember("offsetY", bar.offset.y, document.GetAllocator());

            // Scale
            barComp.AddMember("scaleX", bar.scale.x, document.GetAllocator());
            barComp.AddMember("scaleY", bar.scale.y, document.GetAllocator());

            // Fill Color (vec3) + Alpha
            rapidjson::Value fillColorArray(rapidjson::kArrayType);
            fillColorArray.PushBack(bar.fillColor.r, document.GetAllocator());
            fillColorArray.PushBack(bar.fillColor.g, document.GetAllocator());
            fillColorArray.PushBack(bar.fillColor.b, document.GetAllocator());
            barComp.AddMember("fillColor", fillColorArray, document.GetAllocator());

            barComp.AddMember("fillAlpha", bar.fillAlpha, document.GetAllocator());

            // Background Color (vec3) + Alpha
            rapidjson::Value bgColorArray(rapidjson::kArrayType);
            bgColorArray.PushBack(bar.bgColor.r, document.GetAllocator());
            bgColorArray.PushBack(bar.bgColor.g, document.GetAllocator());
            bgColorArray.PushBack(bar.bgColor.b, document.GetAllocator());
            barComp.AddMember("bgColor", bgColorArray, document.GetAllocator());

            barComp.AddMember("bgAlpha", bar.bgAlpha, document.GetAllocator());

            components.AddMember("UIBarComponent", barComp, document.GetAllocator());
        }

        // Add the components to the entity
        entityObj.AddMember("components", components, document.GetAllocator());

        // Add the entity to the "entities" array
        entities.PushBack(entityObj, document.GetAllocator());
    }

    // Add "entities" array to the document
    document.AddMember("entities", entities, document.GetAllocator());

    std::cout << "Writing JSON to file: " << filename << std::endl;

    // Convert document to JSON string and write to file
    std::ofstream ofs(filename);
    if (!ofs.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << " for writing." << std::endl;
        return;
    }

    rapidjson::OStreamWrapper osw(ofs);
    rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
    document.Accept(writer);

    std::cout << "Entities serialized successfully to " << filename << std::endl;
}

void EntityAsset::DeserializeAnimation(const std::string& filePath)
{
    // Open the file
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return;
    }

    // Read the entire file content into a string
    std::string jsonContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Parse the JSON content using RapidJSON
    rapidjson::Document document;
    document.Parse(jsonContent.c_str());

    if (document.HasParseError())
    {
        std::cerr << "Error parsing JSON file!" << std::endl;
        return;
    }

    // Check if the "animations" array exists
    if (document.HasMember("animations") && document["animations"].IsArray())
    {
        const auto& animationsArray = document["animations"];

        // Loop through each element in the animations array
        for (rapidjson::SizeType i = 0; i < animationsArray.Size(); ++i)
        {
            const auto& animation = animationsArray[i];

            // Ensure each animation has the required fields
            if (animation.HasMember("name") && animation.HasMember("rows") && animation.HasMember("cols") && animation.HasMember("animationSpeed"))
            {
                std::string name = animation["name"].GetString();
                int rows = animation["rows"].GetInt();
                int cols = animation["cols"].GetInt();
                float animationSpeed = animation["animationSpeed"].GetFloat();

                // Create an Animation object
                Animation newAnimation = { rows, cols, animationSpeed };

                // Add the animation to the map
                Framework::GlobalAssetManager.GetAnimationDataMap()[name] = newAnimation;
            }
            else
            {
                std::cerr << "Missing fields in animation data." << std::endl;
            }
        }
    }
    else
    {
        std::cerr << "No animations found in JSON." << std::endl;
    }
}

void EntityAsset::DeserializeBullet(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file)
    {
        std::cerr << "Failed to open bullet data file: " << filePath << std::endl;
        return;
    }

    rapidjson::IStreamWrapper isw(file);
    rapidjson::Document doc;
    doc.ParseStream(isw);

    if (doc.HasParseError())
    {
        std::cerr << "Error parsing bullet data file: " << filePath << std::endl;
        return;
    }

    if (!doc.HasMember("Bullet"))
    {
        std::cerr << "Bullet data missing in JSON file: " << filePath << std::endl;
        return;
    }

    const auto& bulletData = doc["Bullet"];
    BulletData bulletInfo;

    bulletInfo.scale.x = bulletData["scale"]["x"].GetFloat();
    bulletInfo.scale.y = bulletData["scale"]["y"].GetFloat();
    bulletInfo.textureID = bulletData["textureID"].GetString();
    bulletInfo.color = glm::vec3
    (
        bulletData["color"][0].GetFloat(),
        bulletData["color"][1].GetFloat(),
        bulletData["color"][2].GetFloat()
    );
    bulletInfo.alpha = bulletData["alpha"].GetFloat();
    bulletInfo.baseVelocity.x = bulletData["movement"]["baseVelocity"]["x"].GetFloat();
    bulletInfo.baseVelocity.y = bulletData["movement"]["baseVelocity"]["y"].GetFloat();
    bulletInfo.fontName = bulletData["text"]["fontName"].GetString();
    bulletInfo.particleTexture = bulletData["particle"]["textureName"].GetString();
    bulletInfo.particleLife = bulletData["particle"]["life"].GetFloat();
    bulletInfo.particleSize = bulletData["particle"]["size"].GetFloat();
    bulletInfo.particleColor = glm::vec3
    (
        bulletData["particle"]["color"][0].GetFloat(),
        bulletData["particle"]["color"][1].GetFloat(),
        bulletData["particle"]["color"][2].GetFloat()
    );
    bulletInfo.emitDelay = bulletData["particle"]["emitDelay"].GetFloat();
    bulletInfo.emissionRate = bulletData["particle"]["emissionRate"].GetFloat();
    bulletInfo.damageMultiplier = bulletData["damageMultiplier"].GetInt();
    bulletInfo.collisionScale.x = bulletData["CollisionComponent"]["collisionScaleX"].GetFloat();
    bulletInfo.collisionScale.y = bulletData["CollisionComponent"]["collisionScaleY"].GetFloat();

    // Store in AssetManager
    Framework::GlobalAssetManager.StoreBulletData("Bullet", bulletInfo);
    std::cout << "Bullet Data read successfully" << std::endl;
}

std::string EntityAsset::EnemyTypeToString(EnemyType type)
{
    switch (type) 
    {
    case EnemyType::Minion: return "Minion";
    case EnemyType::Boss: return "Boss";
    case EnemyType::Poison: return "Poison";
    case EnemyType::MC: return "MC";
    case EnemyType::Spawner: return "Spawner";
    case EnemyType::Smoke: return "Smoke";
        // Add other cases here
    default: return "Unknown";
    }
}

std::string EntityAsset::ObjectTypeToString(ObjectType type)
{
    switch (type) 
    {
    case ObjectType::Enemy: return "Enemy";
    case ObjectType::CollidableObject: return "CollidableObject";
    case ObjectType::Player: return "Player";
    case ObjectType::Bullet: return "Bullet";
    default: return "Unknown";
    }
}