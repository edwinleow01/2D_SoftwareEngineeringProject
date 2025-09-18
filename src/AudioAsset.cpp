///////////////////////////////////////////////////////////////////////////////
///
///	@File  : AudioAsset.cpp
/// @Brief : Source file implementing the `AudioAsset` class, which manages 
///          the loading, deserialization, and retrieval of audio asset data. 
///          It includes functions for converting playback modes, retrieving 
///          sound information, and handling audio asset paths. The class 
///          interacts with external assets to organize and manage various 
///          audio data for efficient use in the audio system.
///
///	@Main Author : Edwin Leow (100%)
/// @Secondary Author : NIL
///	@Copyright 2024, Digipen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "AudioAsset.h"
#include "Audio.h"

// Deserialize audio assets from a JSON file
void AudioAsset::DeserializeAudio(const std::string& filePath, std::unordered_map<std::string, MusicAsset>& musicAssets)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open JSON file: " << filePath << std::endl;
        throw std::runtime_error("Could not open JSON file.");
    }

    std::stringstream buffer;
    buffer << file.rdbuf(); // Read the entire file into the buffer
    std::string jsonString = buffer.str(); // Get the content as a string

    rapidjson::Document document; // Parse the JSON string
    document.Parse(jsonString.c_str());

    if (document.HasParseError())
    {
        std::cerr << "Error parsing JSON: " << rapidjson::GetParseError_En(document.GetParseError()) << std::endl;
        return;
    }

    if (document.HasMember("musicAssets") && document["musicAssets"].IsArray())
    {
        const rapidjson::Value& musicArray = document["musicAssets"];
        for (rapidjson::SizeType i = 0; i < musicArray.Size(); i++)
        {
            if (musicArray[i].IsObject())
            {
                const rapidjson::Value& musicObject = musicArray[i];
                std::string customName = musicObject["customName"].GetString();
                std::string currentFilePath = musicObject["filePath"].GetString();
                std::string mode = musicObject["mode"].GetString();
                std::string soundTypeStr = musicObject["soundType"].GetString();

                // Create an instance of AudioAsset to call the non-static function
                AudioAsset audioAsset;
                Framework::Audio::SoundType soundType = audioAsset.UE_GetSoundTypeFromString(soundTypeStr);

                // Add to the provided musicAssets map
                musicAssets[customName] = { currentFilePath, mode, soundType };
                
                // Automatically detect randomized sound groups (e.g., "Footstep_01", "Footstep_02")
                //size_t underscorePos = customName.find_last_of('_');
                //if (underscorePos != std::string::npos && underscorePos > 0) // Ensure underscore is not the first character
                //{
                //    std::string baseName = customName.substr(0, underscorePos); // Extract base name (e.g., "Footstep")
                //    Framework::Audio::randomSoundGroups[baseName].push_back(customName);
                //}
                //else
                //{
                //    std::cout << "Warning: customName '" << customName << "' does not contain a valid underscore!" << std::endl;
                //}
            }
        }
    }
    else
    {
        std::cerr << "Invalid JSON structure: 'musicAssets' array not found." << std::endl;
    }
    file.close();
}

// Serialize audio assets to a JSON file
void AudioAsset::SerializeAudio(const std::string& filePath, const std::unordered_map<std::string, MusicAsset>& musicAssets)
{
    rapidjson::Document document;
    document.SetObject();

    rapidjson::Value musicArray(rapidjson::kArrayType);
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    for (const auto& [customName, asset] : musicAssets)
    {
        rapidjson::Value musicObject(rapidjson::kObjectType);

        musicObject.AddMember("customName", rapidjson::Value(customName.c_str(), allocator), allocator);
        musicObject.AddMember("filePath", rapidjson::Value(asset.filePath.c_str(), allocator), allocator);
        musicObject.AddMember("mode", rapidjson::Value(asset.mode.c_str(), allocator), allocator);

        std::string soundTypeStr = SoundTypeToString(asset.soundType);
        musicObject.AddMember("soundType", rapidjson::Value(soundTypeStr.c_str(), allocator), allocator);

        musicArray.PushBack(musicObject, allocator);
    }

    document.AddMember("musicAssets", musicArray, allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    std::ofstream outFile(filePath);
    if (outFile.is_open())
    {
        outFile << buffer.GetString();
        outFile.close();
        std::cout << "Successfully serialized audio assets to " << filePath << std::endl;
    }
    else
    {
        std::cerr << "Error: Could not open file for writing: " << filePath << std::endl;
    }
}

Framework::Audio::SoundType AudioAsset::UE_GetSoundTypeFromString(const std::string& soundTypeStr) const
{
    if (soundTypeStr == "background")
    {
        return Framework::Audio::BACKGROUND_MUSIC;
    }
    else if (soundTypeStr == "effect")
    {
        return Framework::Audio::SOUND_EFFECT;
    }
    else
    {
        throw std::runtime_error("Invalid sound type: " + soundTypeStr);
    }
}

std::string AudioAsset::SoundTypeToString(Framework::Audio::SoundType soundType)
{
    switch (soundType)
    {
    case Framework::Audio::SoundType::BACKGROUND_MUSIC:
        return "background";
    case Framework::Audio::SoundType::EMPTY:
        return "Empty";
    case Framework::Audio::SoundType::SOUND_EFFECT:
        return "effect";
    default:
        return "Unknown";
    }
}
