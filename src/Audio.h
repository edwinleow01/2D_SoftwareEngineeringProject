///////////////////////////////////////////////////////////////////////////////
///
///	@file  : Audio.h
/// @Brief : Header file for Audio system that manages sound loading, playback,
///          channel groups, and volume control using FMOD
///	
///	@Main Author : Edwin Leow (100%)
///	@Secondary Author : NIL
/// @Copyright 2024, Digipen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef _AUDIO_H_
#define _AUDIO_H_

#include "pch.h"                // Contains all required libraries
#include "System.h"             // For Framework
#include "fmod.hpp"				// FMOD_CORE_API
#include "fmod_studio.hpp"		// FMOD_STUDIO_API
#include <unordered_map>        // std::unordered_map
#include <cstdlib>              // For std::rand
#include <ctime>                // For std::time

using namespace std;            // For Standard Library
using namespace FMOD;           // For FMOD Library

namespace Framework 
{
    /**
    *   @class Audio
    *   @brief Manages audio-related tasks such as loading sounds, playing, pausing and adjusting volume
    */
    class Audio : public ISystem 
    {
    public:
        /**
        *   @enum SoundType
        *   @brief Defines types of sounds that can be managed (e.g., background music or sound effects).
        */
        enum SoundType
        {
            EMPTY,              // No sound loaded
            BACKGROUND_MUSIC,   // Background music type
            SOUND_EFFECT        // Sound effect type
        };

        /**
        *   @brief Constructor for Audio system.
        *   @param AssetManager Pointer to the asset manager to load sound assets.
        */
        Audio();

        /**
         * @brief Destructor for Audio system.
         */
        ~Audio();

        /***********************/
        //  General Functions  //
        /***********************/
        /**
         * @brief Initializes the Audio system.
         */
        void Initialize() override;

        /**
         * @brief Updates the Audio system.
         * @param deltaTime Time since the last frame.
         */
        void Update(float deltaTime) override;

        /**
         * @brief Retrieves the system's name.
         * @return The name of the system as a string.
         */
        std::string GetName() override;

        /**********************/
        //  Audio Management  //
        /**********************/
        /**
         * @brief Converts a string representing a mode (e.g., "loop") to an FMOD_MODE.
         * @param mode The mode as a string.
         * @return FMOD_MODE corresponding to the string.
         */
        FMOD_MODE UE_GetModeFromString(const std::string& mode);

        /**
         * @brief Loads a sound by its custom name using the Asset Manager.
         * @param customName The custom name used to load the sound.
         * @return Pointer to the FMOD::Sound object.
         */
        Sound* UE_LoadSound(const std::string& customName);

        /**
         * @brief Plays a sound by its custom name.
         * @param customName The custom name of the sound.
         */
        void UE_PlaySound(const std::string& customName, bool allowMultipleInstances);

        void ClearInactiveChannels();

        /**
         * @brief Pauses a sound that is currently playing.
         * @param customName The custom name of the sound.
         */

        void UE_PauseSound(const std::string& customName);

        /**
         * @brief Sets the volume for a specific sound.
         * @param customName The custom name of the sound.
         * @param volume The volume level to set (0.0f to 1.0f).
         */
        void UE_SetVolume(const std::string& customName, float volume);

        /*******************************/
        //  Channel Group Management  //
        /******************************/
        /**
         * @brief Sets the volume for a specific channel group.
         * @param group Pointer to the channel group.
         * @param volume The volume level to set (0.0f to 1.0f).
         */
        void UE_SetGroupVolume(FMOD::ChannelGroup* group, float volume);

        /**
         * @brief Pauses a specific channel group.
         * @param group Pointer to the channel group to pause.
         */
        void UE_PauseGroup(const std::string& groupName);

        void UE_ResumeGroup(const std::string& groupName);

        void UE_SetGroupPauseState(const std::string& groupName, bool state);

        void UE_ToggleGroupPlayback(const std::string& groupName);

        /**
         * @brief Creates a new channel group for organizing sounds.
         * @param groupName The name of the new channel group.
         */
        void UE_CreateChannelGroup(const std::string& groupName);

        /**********************/
        //  Volume Control    //
        /**********************/
        /**
         * @brief Increases the volume of a specific channel group.
         * @param groupName The name of the channel group.
         */
        void UE_IncrementGroupVol(const std::string& groupName);

        /**
         * @brief Decreases the volume of a specific channel group.
         * @param groupName The name of the channel group.
         */
        void UE_DecrementGroupVol(const std::string& groupName);

        void UE_VolumeControl(const std::string& groupName, bool increase);

        void UE_MuteAllAudio(bool mute);

        /**
         * @brief Pauses all currently playing audio.
         */
        void UE_PauseAllAudio();

        /**
         * @brief Resumes all paused audio.
         */
        void UE_ResumeAllAudio();

        /**
         * @brief Resets the audio system, clearing loaded sounds and active channels.
         */
        void UE_Reset();

        void UE_BGM_Reset();

        //static std::unordered_map<std::string, std::vector<std::string>> randomSoundGroups;

        FMOD::Channel* GetChannel(const std::string& name) 
        {
            auto it = activeChannels.find(name);
            return (it != activeChannels.end()) ? it->second : nullptr;
        }

        FMOD::Sound* GetSound(const std::string& name) 
        {
            auto it = loadedSounds.find(name);
            return (it != loadedSounds.end()) ? it->second : nullptr;
        }

        void UE_CleanupDeadChannels();

        void DebugChannelState()
        {
            std::cout << "=== AUDIO DEBUG ===" << std::endl;

            int playing = 0;
            pSystem->getChannelsPlaying(&playing, nullptr);
            std::cout << "Channels playing: " << playing << std::endl;

            for (auto& pair : activeChannels)
            {
                FMOD::Channel* ch = pair.second;
                if (!ch) continue;

                float vol = 0.0f;
                bool muted = false, paused = false;

                ch->getVolume(&vol);
                ch->getMute(&muted);
                ch->getPaused(&paused);

                std::cout << "Channel [" << pair.first << "] vol=" << vol
                    << " muted=" << muted
                    << " paused=" << paused << std::endl;
            }

            FMOD::ChannelGroup* master = nullptr;
            pSystem->getMasterChannelGroup(&master);
            if (master)
            {
                float masterVol = 0.0f;
                bool masterMute = false;
                master->getVolume(&masterVol);
                master->getMute(&masterMute);

                std::cout << "Master Volume = " << masterVol
                    << ", Muted = " << masterMute << std::endl;
            }

            std::cout << "===================" << std::endl;
        }


    private:
        FMOD::System* pSystem = nullptr;                                            // Create System API, System Object is now a member of the class
        FMOD::ChannelGroup* masterGroup = nullptr;                                  // Declare the Master Group
        std::unordered_map<std::string, FMOD::ChannelGroup*> activeChannelGroup;    // Map of active channel groups
        std::unordered_map<std::string, FMOD::Channel*> activeChannels;             // Map of active channels
        std::unordered_map<std::string, FMOD::Sound*> loadedSounds;                 // Map for storing loaded sounds
        const float volChangeAmount = 0.1f;                                         // Fixed amount to change volume
        
        // Variable to keep track of the next instance ID
        int nextInstanceId = 0;

        // Random pitch generation
        float minPitch = 0.5f;
        float maxPitch = 2.0f;
    };

    extern Audio GlobalAudio;   // Declare a global Audio Instance
}
#endif // _AUDIO_H_