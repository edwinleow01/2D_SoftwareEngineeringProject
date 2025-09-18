///////////////////////////////////////////////////////////////////////////////
///
///	@File  : Audio.cpp
/// @Brief : This file implements the Audio system, which manages audio-related
///          tasks using FMOD. Using asset manager to retrieve sound assets to
///          provide functionality to play and control both background music and
///          sound effects.
///
///	@Main Author : Edwin Leow (100%)
///	@Secondary Author : NIL
/// @Copyright 2024, Digipen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "AssetManager.h"
#include "PlayerSystem.h"

namespace Framework
{
    // Define the global Audio Instance
    Audio GlobalAudio;

    // Constructor
    Audio::Audio()
    {
        System_Create(&pSystem);                                // Create the FMOD System Object
        pSystem->init(64, FMOD_INIT_NORMAL, nullptr);           // Create 32 Channels for 32 Audio
        pSystem->setSoftwareChannels(128);

        pSystem->getMasterChannelGroup(&masterGroup); // Get FMOD's built-in master group

        if (!masterGroup)
        {
            pSystem->createChannelGroup("Master", &masterGroup);
            std::cout << "Master channel group created." << std::endl;
        }

        UE_CreateChannelGroup("BackgroundMusic");           // Create Channel Groups on Initialization
        UE_CreateChannelGroup("SoundEffects");
        UE_CreateChannelGroup("DingSFX");

        // Seed the random number generator (should be done once in your program)
        std::srand(static_cast<unsigned int>(std::time(nullptr)));      // For randomizing pitch
    }

    // Destructor
    Audio::~Audio()
    {
        for (auto& pair : loadedSounds)     // Loop through all loadedSounds
        {
            if (pair.second != nullptr)
            {
                pair.second->release();     // free the sound object
            }
        }
        loadedSounds.clear();               // Clear loaded sounds map
        activeChannels.clear();             // Clear active channels map
        activeChannelGroup.clear();         // Clear active channel groups
        pSystem->release();                 // Free the FMOD System Object
    }

    // Initialize the system
    void Audio::Initialize() {}

    // Update the system
    void Audio::Update(float deltaTime)
    {
        (void)deltaTime;            // Suprress unused parameter warning
        pSystem->update();          // Must keep updating, else audio will not play
        //UE_CleanupDeadChannels();
        //ClearInactiveChannels();
    }

    // Get the name of the system
    std::string Audio::GetName()
    {
        return "Audio";
    }

    // Function to change from String to FMOD_MODE
    FMOD_MODE Audio::UE_GetModeFromString(const std::string& mode)
    {
        if (mode == "loop")
        {
            return FMOD_LOOP_NORMAL;        // Looping Mode
        }
        else if (mode == "oneshot")
        {
            return FMOD_DEFAULT;            // One-shot playback
        }
        else
        {
            throw std::runtime_error("Invalid sound mode : " + mode);
        }
    }

    Sound* Audio::UE_LoadSound(const std::string& customName)
    {
        UE_CleanupDeadChannels();

        // Initialize a Sound pointer to nullptr
        Sound* pSound = nullptr;
        AudioAsset::MusicAsset* audioAsset = GlobalAssetManager.UE_GetAudioAsset(customName);
        AudioAsset::MusicAsset* musicAsset = GlobalAssetManager.UE_GetMusicAssetByName(customName);

        if (!audioAsset)
        {
            std::cout << "Error: AudioAsset not found for customName: " << customName << std::endl;
            return nullptr;
        }

        if (!musicAsset)
        {
            std::cout << "Error: MusicAsset not found for customName: " << customName << std::endl;
            return nullptr;
        }

        std::string filePath = musicAsset->filePath;
        std::string modeString = musicAsset->mode;
        FMOD_MODE mode = UE_GetModeFromString(modeString);                                                  // Convert the string mode to FMOD_MODE
        FMOD_RESULT result = pSystem->createSound(filePath.c_str(), FMOD_IGNORETAGS | mode, 0, &pSound);    // Create Sound

        if (result != FMOD_OK)
        {
            std::cout << "Error, Fail to create audio for " << customName << " " << result << std::endl;
        }

        loadedSounds[customName] = pSound;      // Store the created sound in loadedSounds Map, with customName as key

        return pSound;                          // Return the created sound
    }

    void Audio::UE_PlaySound(const std::string& customName, bool allowMultipleInstances)
    {
        // ClearInactiveChannels();
        // UE_CleanupDeadChannels();

        std::string soundToPlay = customName;
        AudioAsset::MusicAsset* musicAsset = GlobalAssetManager.UE_GetMusicAssetByName(customName);
        Sound* pSound = UE_LoadSound(soundToPlay);
        if (!pSound)
        {
            std::cout << "Error: Sound " << soundToPlay << " could not be loaded." << std::endl;
            return;
        }

        SoundType soundType = musicAsset->soundType;

        std::string channelKey = soundToPlay;
        if (allowMultipleInstances)
        {
            channelKey += "_" + std::to_string(nextInstanceId++); // Unique ID for multiple instances
        }
        else
        {
            auto it = activeChannels.find(soundToPlay);
            if (it != activeChannels.end() && it->second)
            {
                bool isPlaying = false;
                it->second->isPlaying(&isPlaying);
                if (isPlaying) return;

                bool isPaused = false;
                it->second->getPaused(&isPaused);
                if (isPaused)
                {
                    it->second->setPaused(false);
                    return;
                }
                activeChannels.erase(it); // Remove finished sound
            }
        }

        FMOD::Channel* pChannel = nullptr;
        FMOD_RESULT result = pSystem->playSound(pSound, nullptr, false, &pChannel);
        if (result != FMOD_OK)
        {
            std::cout << "Error, cannot play audio";
            return;
        }

        if (pChannel)
        {
            if (soundType == BACKGROUND_MUSIC)
            {
                pChannel->setChannelGroup(activeChannelGroup["BackgroundMusic"]);
            }
            
            if (soundType == SOUND_EFFECT && !(customName.rfind("Ding", 0) == 0))
            {
                pChannel->setChannelGroup(activeChannelGroup["SoundEffects"]);
            }
            
            if (customName.rfind("Ding", 0) == 0)
            {
                //pChannel->setVolume(0.7f);
                pChannel->setChannelGroup(activeChannelGroup["DingSFX"]);
            }

            activeChannels[channelKey] = pChannel;
        }

        //DebugChannelState();
    }

    void Audio::ClearInactiveChannels()
    {
        // Iterate through active channels and check if they're still playing
        for (auto it = activeChannels.begin(); it != activeChannels.end(); )
        {
            FMOD::Channel* channel = it->second;
            if (channel)
            {
                bool isPlaying = false;
                channel->isPlaying(&isPlaying);

                if (!isPlaying)
                {
                    it = activeChannels.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }

    void Audio::UE_PauseSound(const std::string& customName)
    {
        auto it = activeChannels.find(customName);      // Find the Channel based on the customName
        if (it != activeChannels.end())
        {
            bool isPaused;
            it->second->getPaused(&isPaused);           // Get the current paused state
            it->second->setPaused(!isPaused);           // Toggle the paused state
            std::cout << (isPaused ? "Resuming " : "Pausing ") << customName << std::endl;
        }
        else
        {
            std::cerr << "Error: Channel for " << customName << " not found!" << std::endl;
        }
    }

    void Audio::UE_SetVolume(const std::string& customName, float volume)
    {
        auto it = activeChannels.find(customName);              // Find the channel associated with the custom sound name

        if (it != activeChannels.end() && it->second != nullptr)
        {
            FMOD::Channel* pChannel = it->second;               // Retrieve the Channel from customName 
            pChannel->setVolume(volume);                        // Set the volume on the channel (0.0f is silent, 1.0f is max volume)
            std::cout << "Volume for " << customName << " set to " << volume << std::endl;
        }
        else
        {
            std::cerr << "Error: Channel for " << customName << " not found!" << std::endl;
        }
    }

    void Audio::UE_SetGroupVolume(FMOD::ChannelGroup* group, float volume)
    {
        if (group != nullptr)
        {
            group->setVolume(volume);       // Set the volume of the group to the given volume
            std::cout << "Set volume for channel group to " << volume << std::endl;
        }
    }

    void Audio::UE_PauseGroup(const std::string& groupName)
    {
        auto it = activeChannelGroup.find(groupName);  // Find the group by its name
        if (it != activeChannelGroup.end() && it->second != nullptr)
        {
            FMOD::ChannelGroup* group = it->second;
            bool isPaused;
            group->getPaused(&isPaused);    // Retrieve the pause state for the group
            group->setPaused(!isPaused);    // Toggle pause state for the group
            std::cout << (isPaused ? "Resuming group: " : "Pausing group: ") << groupName << std::endl;
        }
        else
        {
            std::cout << "Error: Channel group '" << groupName << "' not found or is nullptr." << std::endl;
        }
    }

    void Audio::UE_ResumeGroup(const std::string& groupName)
    {
        // Find the group by its name
        auto it = activeChannelGroup.find(groupName);
        if (it != activeChannelGroup.end() && it->second != nullptr)
        {
            FMOD::ChannelGroup* group = it->second;

            // Set the paused state to false
            group->setPaused(false);
            std::cout << "Resumed group: " << groupName << std::endl;
        }
        else
        {
            std::cout << "Error: Channel group '" << groupName << "' not found or is nullptr." << std::endl;
        }
    }

    void Audio::UE_SetGroupPauseState(const std::string& groupName, bool state)
    {
        auto it = activeChannelGroup.find(groupName);
        if (it != activeChannelGroup.end() && it->second != nullptr)
        {
            it->second->setPaused(state);
            std::cout << (state ? "Paused group: " : "Resumed group: ") << groupName << std::endl;
        }
        else
        {
            std::cout << "Error: Channel group '" << groupName << "' not found or is nullptr." << std::endl;
        }
    }

    void Audio::UE_ToggleGroupPlayback(const std::string& groupName)
    {
        auto it = activeChannelGroup.find(groupName);  // Find the group by name
        if (it != activeChannelGroup.end() && it->second != nullptr)
        {
            FMOD::ChannelGroup* group = it->second;

            bool isPaused = false;
            group->getPaused(&isPaused);    // Check the current pause state
            group->setPaused(!isPaused);   // Toggle pause state

            if (isPaused)
            {
                std::cout << "Resumed group: " << groupName << std::endl;
            }
            else
            {
                // Additional stop logic can be added here if needed
                std::cout << "Paused group: " << groupName << std::endl;
            }
        }
        else
        {
            std::cout << "Error: Channel group '" << groupName << "' not found or is nullptr." << std::endl;
        }
    }

    void Audio::UE_CreateChannelGroup(const std::string& groupName)
    {
        if (activeChannelGroup.find(groupName) != activeChannelGroup.end())
        {
            std::cout << "Channel group '" << groupName << "' already exists." << std::endl;
            return;
        }

        FMOD::ChannelGroup* newGroup = nullptr;
        pSystem->createChannelGroup(groupName.c_str(), &newGroup);      //  Create a new ChannelGroup with given groupName

        if (newGroup)
        {
            if (groupName == "BackgroundMusic")
            {
                newGroup->setVolume(0.7f);
            }

            if (groupName == "DingSFX")                                 // Storing ONLY Dings SFX
            {
                newGroup->setVolume(0.3f);
            }

            activeChannelGroup[groupName] = newGroup;                   // Store newGroup in Map of activeChannelGroup

            // If master group exists, attach this new group to it
            if (masterGroup)
            {
                masterGroup->addGroup(newGroup);
            }
        }
    }

    void Audio::UE_IncrementGroupVol(const std::string& groupName)
    {
        auto it = activeChannelGroup.find(groupName);               // Search for GroupName
        if (it != activeChannelGroup.end())                         // Iterate through the activeChannelGroups
        {
            FMOD::ChannelGroup* group = it->second;                 // Retrieve the group

            float currentVolume;                                    // Create float to hold currentVolume
            FMOD_RESULT result = group->getVolume(&currentVolume);  // Retrieve the currentVolume of the group
            if (result == FMOD_OK)
            {
                float newVolume = currentVolume + volChangeAmount;  // Increment the group's volume
                newVolume = (newVolume > 1.0f) ? 1.0f : newVolume;  // Ensure volume does not exceed 1.0 (max volume)
                group->setVolume(newVolume);                        // Set the group with the newVolume
                std::cout << "Increased volume of " << groupName << " to " << newVolume << std::endl;
            }
            else
            {
                std::cout << "Error getting volume for group " << groupName << std::endl;
            }
        }
        else
        {
            std::cout << "Channel group " << groupName << " not found." << std::endl;
        }
    }

    void Audio::UE_DecrementGroupVol(const std::string& groupName)
    {
        auto it = activeChannelGroup.find(groupName);               // Search for groupName
        if (it != activeChannelGroup.end())                         // Iterate through the activeChannelGroups
        {
            FMOD::ChannelGroup* group = it->second;                 // Retrieve the group

            float currentVolume;                                    // Create float to hold currentVolume
            FMOD_RESULT result = group->getVolume(&currentVolume);  // Retrieve the currentVolume of the group          
            if (result == FMOD_OK)
            {
                float newVolume = currentVolume - volChangeAmount;  // Decrement the group's volume

                newVolume = (newVolume < 0.0f) ? 0.0f : newVolume;  // Ensure volume does not fall below 0.0 (min volume)
                group->setVolume(newVolume);                        // Set the group with the newVolume

                std::cout << "Decreased volume of " << groupName << " to " << newVolume << std::endl;
            }
            else
            {
                std::cout << "Error getting volume for group " << groupName << std::endl;
            }
        }
        else
        {
            std::cout << "Channel group " << groupName << " not found." << std::endl;
        }
    }

    void Audio::UE_VolumeControl(const std::string& groupName, bool increase)
    {
        FMOD::ChannelGroup* group = nullptr;

        if (groupName == "Master" && masterGroup)
        {
            group = masterGroup; // Use Master Group
        }
        else
        {
            auto it = activeChannelGroup.find(groupName);
            if (it != activeChannelGroup.end())
            {
                group = it->second; // Use the specified group
            }
        }

        if (group)
        {
            float currentVolume;
            if (group->getVolume(&currentVolume) == FMOD_OK)
            {
                if (groupName == "BackgroundMusic")
                {
                    float newVolume = static_cast<float>(increase ? (currentVolume + 0.07) : (currentVolume - 0.07));
                    newVolume = std::clamp(newVolume, 0.0f, 1.0f); // Ensure volume stays between 0.0 and 1.0

                    group->setVolume(newVolume);
                    std::cout << (increase ? "Increased" : "Decreased")
                        << " volume of " << groupName
                        << " to " << newVolume << std::endl;
                }
                else if (groupName == "DingSFX")
                {
                    float newVolume = static_cast<float>(increase ? (currentVolume + 0.03) : (currentVolume - 0.03));
                    newVolume = std::clamp(newVolume, 0.0f, 1.0f); // Ensure volume stays between 0.0 and 1.0

                    group->setVolume(newVolume);
                    std::cout << (increase ? "Increased" : "Decreased")
                        << " volume of " << groupName
                        << " to " << newVolume << std::endl;
                }
                else
                {
                    float newVolume = increase ? (currentVolume + volChangeAmount) : (currentVolume - volChangeAmount);
                    newVolume = std::clamp(newVolume, 0.0f, 1.0f); // Ensure volume stays between 0.0 and 1.0

                    group->setVolume(newVolume);
                    std::cout << (increase ? "Increased" : "Decreased")
                        << " volume of " << groupName
                        << " to " << newVolume << std::endl;
                }

                // If adjusting Master Volume, reapply scaling to child groups
                if (group == masterGroup)
                {
                    float newVolume = increase ? (currentVolume + volChangeAmount) : (currentVolume - volChangeAmount);
                    newVolume = std::clamp(newVolume, 0.0f, 1.0f); // Ensure volume stays between 0.0 and 1.0

                    for (auto& [name, childGroup] : activeChannelGroup)         // Master Group acts as a multiplier to all groups
                    {
                        float childVolume;
                        if (childGroup->getVolume(&childVolume) == FMOD_OK)
                        {
                            float scaledVolume = childVolume * newVolume;       // Apply Master scaling
                            childGroup->setVolume(scaledVolume);
                            std::cout << "Adjusted " << name << " volume to " << scaledVolume << " (scaled by Master)\n";
                        }
                    }
                }
            }
            else
            {
                std::cout << "Error getting volume for group " << groupName << std::endl;
            }
        }
        else
        {
            std::cout << "Channel group " << groupName << " not found." << std::endl;
        }
    }

    void Audio::UE_MuteAllAudio(bool mute)
    {
        if (masterGroup)
        {
            masterGroup->setMute(mute);
            std::cout << (mute ? "Muted" : "Unmuted") << " all audio using setMute().\n";
        }
    }

    void Audio::UE_PauseAllAudio()
    {
        for (auto& channelEntry : activeChannels)   // Iterate through all active channel
        {
            FMOD::Channel* pChannel = channelEntry.second;
            if (pChannel != nullptr)
            {
                bool isPlaying = false;
                pChannel->isPlaying(&isPlaying);    // Check if channel is playing
                if (isPlaying)
                {
                    pChannel->setPaused(true);      // Pause the channel if its playing
                }
            }
        }
    }

    void Audio::UE_ResumeAllAudio()
    {
        for (auto& channelEntry : activeChannels)  // Iterate through all active channels
        {
            FMOD::Channel* pChannel = channelEntry.second;
            if (pChannel != nullptr)
            {
                bool isPaused = false;
                pChannel->getPaused(&isPaused);    // Check if the channel is paused
                if (isPaused)
                {
                    pChannel->setPaused(false);    // Unpause the channel if it's paused
                }
            }
        }
    }

    void Audio::UE_Reset()
    {
        UE_PauseAllAudio();                    // Pause any current playing audio

        // Iterate through active channels and stop/rewind each
        for (auto& pair : activeChannels)
        {
            FMOD::Channel* channel = pair.second;
            if (channel != nullptr)
            {
                channel->stop();  // Stop the sound
                //channel->setPosition(0, FMOD_TIMEUNIT_MS);  // Rewind to the beginning
            }
        }

        activeChannels.clear();  // Clear active channels but keep loaded sounds
        
        //UE_CleanupDeadChannels();

        std::cout << "Audio system has been reset by stopping and rewinding sounds." << std::endl;
    }

    void Audio::UE_BGM_Reset()
    {
        // Retrieve the BackgroundMusic group
        auto it = activeChannelGroup.find("BackgroundMusic");
        if (it == activeChannelGroup.end() || it->second == nullptr)
        {
            std::cout << "Error: BackgroundMusic group not found or is nullptr." << std::endl;
            return;
        }

        FMOD::ChannelGroup* bgmGroup = it->second;

        // Iterate through active channels and reset only those in the BackgroundMusic group
        for (auto& pair : activeChannels)
        {
            FMOD::Channel* channel = pair.second;
            if (channel != nullptr)
            {
                FMOD::ChannelGroup* channelGroup = nullptr;
                channel->getChannelGroup(&channelGroup);

                // Check if the channel belongs to the "BackgroundMusic" group
                if (channelGroup == bgmGroup)
                {
                    channel->stop();                   // Stop the channel
                    //channel->setPosition(0, FMOD_TIMEUNIT_MS); // Rewind to the beginning
                }
            }
        }

        //UE_CleanupDeadChannels();

        std::cout << "Background music group has been reset." << std::endl;
    }

    void Audio::UE_CleanupDeadChannels()
    {
        for (auto it = activeChannels.begin(); it != activeChannels.end();)
        {
            bool isPlaying = false;
            if (it->second && it->second->isPlaying(&isPlaying) == FMOD_OK && !isPlaying)
            {
                it = activeChannels.erase(it); // Remove dead channel
            }
            else
            {
                ++it;
            }
        }
    }
}