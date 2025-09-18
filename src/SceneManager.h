#pragma once
///////////////////////////////////////////////////////////////////////////////
///
/// @file SceneManager.h
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
#pragma once //Makes sure this header is only included once

#include "pch.h"
#include "System.h"
#include "Vector2D.h"
#include "InputHandler.h"
#include "ComponentList.h"
#include "Audio.h"


#pragma once
#include <string>
namespace Framework {

    class SceneManager : public ISystem {
    public:
        SceneManager();
        ~SceneManager();

        void Initialize() override;               // Override Initialize from ISystem
        void Update(float deltaTime) override;    // Override Update from ISystem
        std::string GetName() override;           // Override GetName from ISystem

        void TransitionToScene(const std::string& sceneName); // Trigger scene transition
        bool IsSceneTransitioning() const;        // Check if a transition is in progress

        void LoadScene(const std::string& sceneName); // Load a new scene
        void SaveScene(const std::string& filename); // save a scene with filename
        
        //Game scene transitions with logic
        void LoadMenu();

        void ResetLevelVars();

        static std::string Variable_Scene;

        std::string currentScene;
    private:
        std::string nextScene;
        bool sceneTransitionFlag = false;
        bool hasPlayedMenuAudio = false; // Add this flag to track if the audio has been played
        bool hasPlayedGameLevelAudio = false;

        void ClearCurrentScene();                 // Clear all entities in the current scene
    };

    extern SceneManager GlobalSceneManager;
} // namespace Framework
