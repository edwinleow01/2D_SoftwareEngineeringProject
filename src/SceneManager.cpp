///////////////////////////////////////////////////////////////////////////////
///
/// @file SceneManager.cpp
/// 
/// @brief Implements the SceneManager class, which manages scenes within the 
///        game engine. This class handles scene transitions, including clearing 
///        the current scene, loading a new scene, and managing scene-specific 
///        assets and audio.
///
/// @authors 
/// Joshua Sim Yue Chen, Edwin Leow
/// 
/// @copyright 
/// 2024, Digipen Institute of Technology
///////////////////////////////////////////////////////////////////////////////


#include "pch.h"
#include "EntityManager.h"
#include "SceneManager.h"
#include "Coordinator.h"
#include <iostream>
#include "AssetManager.h"
#include "EngineState.h"
#include "PlayerSystem.h"

extern Framework::Coordinator ecsInterface;

namespace Framework {

    SceneManager GlobalSceneManager;

    SceneManager::SceneManager() {
        // Constructor logic if needed
    }

    SceneManager::~SceneManager() {
        // Destructor logic if needed
    }

    void SceneManager::Initialize() {
        GlobalSceneManager.currentScene = "DefaultScene";
        GlobalSceneManager.nextScene = "";
        GlobalSceneManager.sceneTransitionFlag = false;

        std::cout << "SceneManager initialized with DefaultScene." << std::endl;
    }

    // reset all vars, called everytime scene change.
    void SceneManager::ResetLevelVars() {
        engineState.SlowCount = 3; 
        engineState.bombCount = 3;
        engineState.player_health = engineState.player_maxHealth;
        Framework::PlayerSystem::playerText.clear();
        engineState.TimeScale = 1.f; // Reset timescale to default in case of slow
        engineState.winCheat = false;
    }


    void SceneManager::Update(float deltaTime) {

        (void)deltaTime;
 
        // GlobalAudio.ClearInactiveChannels();
        GlobalAudio.UE_CleanupDeadChannels();

        // Audio management for game-specific scenes
        if (engineState.IsPlay()) {

            if (!engineState.IsPaused())
            {
                GlobalAudio.UE_ResumeAllAudio();
            }

            if ((GlobalSceneManager.currentScene == "Assets/Scene/MenuScene.json" ||
                GlobalSceneManager.currentScene == "Assets/Scene/EditorInstance.json" ||
                GlobalSceneManager.currentScene == "Assets/Scene/HowToPlayScene.json" ||
                GlobalSceneManager.currentScene == "Assets/Scene/Credits.json") &&
                !hasPlayedMenuAudio)
            {
                Framework::GlobalAudio.UE_Reset();
                Framework::GlobalAudio.UE_PlaySound("MainMenu_BGM", false); 
                hasPlayedMenuAudio = true; // Set flag to true to prevent re-playing
            }

            else if     ((GlobalSceneManager.currentScene == "Assets/Scene/GameLevel.json" ||
                        GlobalSceneManager.currentScene == "Assets/Scene/BossLevel_Final_Updated.json" ||
                        GlobalSceneManager.currentScene == "Assets/Scene/HardLevel_Final_Updated.json" ||
                        GlobalSceneManager.currentScene == "Assets/Scene/EasyLevel_Final_Updated.json") &&
                        !hasPlayedGameLevelAudio)
            {
                Framework::GlobalAudio.UE_Reset();
                Framework::GlobalAudio.UE_PlaySound("Music_Level_BGM", false);
                hasPlayedGameLevelAudio = true; // Set flag to true to prevent re-playing
            }
        }

        if (GlobalSceneManager.sceneTransitionFlag) {

            // Clear the current scene
            GlobalSceneManager.ClearCurrentScene();

            // Load the next scene
            GlobalSceneManager.LoadScene(GlobalSceneManager.nextScene);

            // Reset the flag
            GlobalSceneManager.sceneTransitionFlag = false;

            std::cout << "Scene transitioned to: "
                << GlobalSceneManager.currentScene
                << std::endl;

            // Reset only when transitioning to a different scene where audio is played
            if (GlobalSceneManager.currentScene != "Assets/Scene/MenuScene.json" &&
                GlobalSceneManager.currentScene != "Assets/Scene/EditorInstance.json" &&
                GlobalSceneManager.currentScene != "Assets/Scene/HowToPlayScene.json" &&
                GlobalSceneManager.currentScene != "Assets/Scene/Credits.json")
            {
                hasPlayedMenuAudio = false;
            }

            if (GlobalSceneManager.currentScene != "Assets/Scene/GameLevel.json" ||
                GlobalSceneManager.currentScene != "Assets/Scene/BossLevel_Final_Updated.json" ||
                GlobalSceneManager.currentScene != "Assets/Scene/HardLevel_Final_Updated.json" ||
                GlobalSceneManager.currentScene != "Assets/Scene/EasyLevel_Final_Updated.json")
            {
                hasPlayedGameLevelAudio = false;
            }
        }
    }

    std::string SceneManager::GetName() {
        return "SceneManager";
    }

    void SceneManager::TransitionToScene(const std::string& sceneName) {
        ResetLevelVars(); // reset level variables everytime a scene is changed.
        GlobalSceneManager.nextScene = sceneName;
        GlobalSceneManager.sceneTransitionFlag = true;
        std::cout << "SceneManager flagged transition to scene: " << nextScene  << " FLAG: " << GlobalSceneManager.sceneTransitionFlag << std::endl;
    }

    bool SceneManager::IsSceneTransitioning() const {
        return GlobalSceneManager.sceneTransitionFlag;
    }

    void SceneManager::ClearCurrentScene() {
        ecsInterface.ClearEntities();
        std::cout << "Cleared all entities for scene transition." << std::endl;
    }

    void SceneManager::LoadScene(const std::string& sceneName) {

        GlobalAssetManager.UE_LoadEntities(sceneName); // Temporarily load this scene
        GlobalSceneManager.currentScene = sceneName;
        std::cout << "Loaded scene: " << GlobalSceneManager.currentScene << std::endl;
    }

    void SceneManager::SaveScene(const std::string& filename) {
        GlobalEntityAsset.SerializeEntities(filename);
    }

    void SceneManager::LoadMenu() {
        // Initialize default scene
        Framework::GlobalSceneManager.TransitionToScene("Assets/Scene/StartScreenTransition.json");
    }

} // namespace Framework