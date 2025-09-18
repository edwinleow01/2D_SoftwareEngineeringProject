///////////////////////////////////////////////////////////////////////////////
///
///	@File  : AudioAsset.h
/// @Brief : Header file defining the `AudioAsset` class, which manages audio
///          asset data. This class includes functionalities for loading, 
///          serializing, and retrieving audio asset information, including 
///          file paths, playback modes, and sound types. It is used to 
///          handle and organize audio assets within the application.
///          The class stores audio information in a structured format for 
///          easy retrieval and management.
///
///	@Main Author : Edwin Leow (100%)
/// @Secondary Author : NIL
///	@Copyright 2024, Digipen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef _AUDIO_ASSET_H_
#define _AUDIO_ASSET_H_
#include "pch.h"
#include "JsonSerialize.h"
#include "Audio.h"

class AudioAsset
{
public:

    /**
        *   @struct MusicAsset
        *   @brief Struct to store music asset data including file path, mode, and
        *          sound type.
        */
    struct MusicAsset
    {
        std::string filePath;                                                                   // Path to music file
        std::string mode = "oneshot";                                                           // Playback mode
        Framework::Audio::SoundType soundType = Framework::Audio::SoundType::SOUND_EFFECT;      // Type of sound
    };

    AudioAsset() = default;;

    /**
     * @brief Constructor that initializes the AudioAsset with the given file path.
     * @param filePath Path to the audio file.
     */
    AudioAsset(const std::string& filePath) : filePath(filePath) {};
    
    /**
     * @brief Destructor for AudioAsset. Cleans up any resources used by the audio assets.
     */
    ~AudioAsset() = default;

    /**
     * @brief Deserializes the audio asset data from a specified file path.
     * @param filePath Path to the audio asset file containing JSON or other serialized data.
     */
    static void DeserializeAudio(const std::string& filePath, std::unordered_map<std::string, MusicAsset>& musicAssets);

    static void SerializeAudio(const std::string& filePath, const std::unordered_map<std::string, MusicAsset>& musicAssets);

    /**
     * @brief Converts a string representing a sound type to its corresponding enumeration value.
     * @param soundTypeStr The string representing the sound type (e.g., "music", "sfx").
     * @return The corresponding SoundType enumeration value.
     */
    Framework::Audio::SoundType UE_GetSoundTypeFromString(const std::string& soundTypeStr) const;
    
    static std::string SoundTypeToString(Framework::Audio::SoundType soundType);

private:
    std::string filePath; // Path to the file for serialization/deserialization
};
#endif // !_AUDIO_ASSET_H_