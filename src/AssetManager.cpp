///////////////////////////////////////////////////////////////////////////////
///
///	@File  : AssetManager.cpp
/// @Brief : Implementation of the AssetManager class, which provides 
///          functionality to load, manage, and retrieve various asset types, 
///          including audio, textures, fonts, and shaders. This file contains 
///          definitions for all the methods declared in AssetManager.h. 
///          The class supports efficient resource reuse, name updates, and 
///          file management operations.
///
///	@Main Author : Edwin Leow (100%)
/// @Secondary Author : NIL
///	@Copyright 2024, Digipen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "AssetManager.h"
#include "LogicManager.h"
#include "FontSystem.h"
#include <iostream>
#include <filesystem>
#include <string>

namespace Framework
{
    AssetManager GlobalAssetManager;        // Global instance of AssetManager.
    unsigned char* AssetManager::data{};    // Texture data buffer for loading textures.

    AssetManager::AssetManager()
    {
        // Initialization of Assets
        UE_LoadAudio("Assets/JsonData/AudioAsset.json");
        UE_LoadTexture("Assets/JsonData/TextureAsset.json");
    }

    Window& AssetManager::UE_LoadWindow(const std::string& filePath)
    {
        // Check if the window is already loaded
        auto it = windowAssets.find(filePath);
        if (it != windowAssets.end())
        {
            return *(it->second); // Return existing window if already loaded
        }

        auto window = std::make_unique<Window>(filePath);
        Window& ref = *window;	// Ref to the created window

        // store the unique_ptr in the map
        windowAssets[filePath] = std::move(window);	// move ownership to map

        return ref;		// Return a ref to the window
    }

    void AssetManager::UE_LoadDictionary(const std::string& fileName) {
        std::ifstream file(fileName);
        if (!file.is_open()) {
            std::cerr << "Could not open the words file!" << std::endl;
            return;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        std::string jsonString = buffer.str();
        size_t keyPos = jsonString.find("\"words\":");
        if (keyPos == std::string::npos) {
            std::cerr << "Key \"words\" not found in JSON!" << std::endl;
            return;
        }

        size_t arrayStart = jsonString.find('[', keyPos);
        size_t arrayEnd = jsonString.find(']', arrayStart);
        if (arrayStart == std::string::npos || arrayEnd == std::string::npos) {
            std::cerr << "Invalid JSON array format!" << std::endl;
            return;
        }

        std::string arrayContent = jsonString.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
        std::stringstream ss(arrayContent);
        std::string word;

        dictionaryWords.clear();  // Clear previously loaded words

        Framework::Trie& trie = Framework::Lexicon::GetInstance()->GetTrie();

        while (std::getline(ss, word, ',')) {
            word.erase(remove(word.begin(), word.end(), '"'), word.end());      // Remove quotes
            word = trim(word);                                                  // Trim spaces
            std::transform(word.begin(), word.end(), word.begin(), ::tolower);  // To lowercase

            if (!word.empty()) {
                dictionaryWords.push_back(word);  // Store sanitized word
                trie.insert(word);
            }
        }
    }

    void AssetManager::UE_LoadPrefixes(const std::string& fileName) {
        std::ifstream file(fileName);
        if (!file.is_open()) {
            std::cerr << "Could not open the prefixes file!" << std::endl;
            return;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        std::string jsonString = buffer.str();
        size_t keyPos = jsonString.find("\"prefixes\":");
        if (keyPos == std::string::npos) {
            std::cerr << "Key \"prefixes\" not found in JSON!" << std::endl;
            return;
        }

        size_t arrayStart = jsonString.find('[', keyPos);
        size_t arrayEnd = jsonString.find(']', arrayStart);
        if (arrayStart == std::string::npos || arrayEnd == std::string::npos) {
            std::cerr << "Invalid JSON array format!" << std::endl;
            return;
        }

        std::string arrayContent = jsonString.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
        std::stringstream ss(arrayContent);
        std::string item;

        prefixList.clear();  // Clear previously loaded prefixes

        while (std::getline(ss, item, ',')) {
            item.erase(remove(item.begin(), item.end(), '"'), item.end());  // Remove quotes
            item = trim(item);                                              // Trim spaces

            if (!item.empty()) {
                prefixList.push_back(item);  // Add to prefixes
            }
        }
    }

    void AssetManager::UE_LoadNSFW(const std::string& fileName)
    {
        std::ifstream file(fileName);
        if (!file.is_open()) 
        {
            std::cerr << "Could not open the NSFW words file: " << fileName << std::endl;
            return;
        }

        // Read the file contents into a string buffer
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        // Find the "nsfw" key in the JSON string
        std::string jsonString = buffer.str();
        size_t keyPos = jsonString.find("\"nsfw\":");
        if (keyPos == std::string::npos) 
        {
            std::cerr << "Key \"nsfw\" not found in JSON!" << std::endl;
            return;
        }

        // Locate the array start and end brackets
        size_t arrayStart = jsonString.find('[', keyPos);
        size_t arrayEnd = jsonString.find(']', arrayStart);
        if (arrayStart == std::string::npos || arrayEnd == std::string::npos) 
        {
            std::cerr << "Invalid JSON array format for key \"nsfw\"!" << std::endl;
            return;
        }

        // Extract the array content
        std::string arrayContent = jsonString.substr(arrayStart + 1, arrayEnd - arrayStart - 1);

        // Parse the array and process each item
        std::stringstream ss(arrayContent);
        std::string item;

        Framework::Trie& nsfwTrie = Framework::Lexicon::GetInstance()->GetNSFW();

        nsfwList.clear(); // Clear the list if storing words there too

        while (std::getline(ss, item, ',')) 
        {
            // Remove surrounding quotes
            item.erase(remove(item.begin(), item.end(), '"'), item.end());

            // Trim spaces
            item.erase(item.begin(), std::find_if(item.begin(), item.end(), [](unsigned char ch) 
                {
                return !std::isspace(ch);
                }));
            item.erase(std::find_if(item.rbegin(), item.rend(), [](unsigned char ch) 
                {
                return !std::isspace(ch);
                }).base(), item.end());

            // Convert to lowercase
            std::transform(item.begin(), item.end(), item.begin(), ::tolower);

            // Insert into trie or list if not empty
            if (!item.empty()) 
            {
                nsfwTrie.insert(item); // Insert into NSFW Trie
                nsfwList.push_back(item); // Optional: Store in a list
            }
        }
    }

    void AssetManager::UE_LoadEntities(const std::string& filePath)
    {
        // If not, load it and store it in the container
        auto entityAsset = std::make_unique<EntityAsset>(filePath);
        entityAssets[filePath] = std::move(entityAsset);
    }

    void AssetManager::UE_LoadPrefab(const std::string& prefabName, glm::vec2 Location)
    {
        if (!prefabName.empty())
        {
            // Get the current working directory (expected to be in 'src/')
            std::string workingDir = std::filesystem::current_path().string();

            // Go up one directory from 'src/' to project root and enter 'Assets/Prefabs/'
            std::string prefabPath = workingDir + "/Assets/Prefabs/" + prefabName;

            // Normalize the path to avoid issues (C++17 feature)
            prefabPath = std::filesystem::weakly_canonical(prefabPath).string();
   
            // Load the prefab
                // If not, load it and store it in the container
            auto entityAsset = std::make_unique<EntityAsset>(prefabPath, Location);
            entityAssets[prefabPath] = std::move(entityAsset);

            std::cout << "Loaded Prefab: " << prefabPath << std::endl;
        }
    }



    void AssetManager::UE_LoadAudio(const std::string& filePath)
    {
        // Call the static method to deserialize the audio data and populate audioAssets
        AudioAsset::DeserializeAudio(filePath, audioAssets);
    }

    AudioAsset::MusicAsset* AssetManager::UE_GetAudioAsset(const std::string& assetName)
    {
        auto it = audioAssets.find(assetName);
        if (it != audioAssets.end())
        {
            return &it->second;  // Returning reference to MusicAsset
        }
        return nullptr;  // If the asset is not found
    }

    void AssetManager::UE_AddAudio(const std::string& path)
    {
        // Define the base folder for audio files
        std::string targetFolder = "Assets/Audio/bgm/";  // Fixed folder for storing audio

        // Extract the file name from the source path (with extension)
        size_t pos = path.find_last_of("/\\");
        std::string fileNameWithExtension = path.substr(pos + 1);  // Get only the file name with extension

        // Remove the file extension from the file name to get just the name (without extension)
        size_t dotPos = fileNameWithExtension.find_last_of(".");
        std::string fileNameWithoutExtension = (dotPos != std::string::npos) ? fileNameWithExtension.substr(0, dotPos) : fileNameWithExtension;

        // Construct the target path (with extension)
        std::string targetPath = targetFolder + fileNameWithExtension;  // Full path in the target folder with extension

        // Copy the audio to the target folder
        if (!UE_CopyAudioToFolder(path, targetFolder))
        {
            std::cerr << "Failed to copy audio to target folder." << std::endl;
            return;
        }
        std::cout << "Audio successfully copied to target folder." << std::endl;

        // Check if the audio already exists in the map using the name (without extension)
        auto it = audioAssets.find(fileNameWithoutExtension);  // Use the name without extension as the key
        if (it == audioAssets.end())
        {
            // If the AudioAsset doesn't exist, create a new MusicAsset
            AudioAsset::MusicAsset newMusicAsset;
            newMusicAsset.filePath = targetPath;  // Store the full path with extension
            newMusicAsset.mode = "oneshot";  // Default playback mode
            newMusicAsset.soundType = Framework::Audio::SoundType::SOUND_EFFECT;  // Default sound type

            // Add the new MusicAsset to the audioAssets map using the name without extension
            audioAssets[fileNameWithoutExtension] = newMusicAsset;  // Directly insert the MusicAsset

            // Serialize the MusicAsset (now part of AudioAsset)
            AudioAsset::SerializeAudio("Assets/JsonData/AudioAsset.json", audioAssets);
        }
        else
        {
            // If the MusicAsset already exists, just update it
            AudioAsset::MusicAsset newMusicAsset;
            newMusicAsset.filePath = targetPath;  // Store the full path with extension
            newMusicAsset.mode = "oneshot";  // Default playback mode
            newMusicAsset.soundType = Framework::Audio::SoundType::SOUND_EFFECT;  // Default sound type

            it->second = newMusicAsset;  // Update the existing MusicAsset in the map
        }
    }

    bool AssetManager::UE_CopyAudioToFolder(const std::string& sourceFilePath, const std::string& targetFolder)
    {
        // Extract the filename from the source path
        size_t pos = sourceFilePath.find_last_of("/\\");
        std::string fileName = sourceFilePath.substr(pos + 1);

        // Construct the target file path
        std::string targetFilePath = targetFolder + "\\" + fileName;

        // Open the source file for reading in binary mode
        std::ifstream sourceFile(sourceFilePath, std::ios::binary);
        if (!sourceFile.is_open())
        {
            std::cerr << "Failed to open source file: " << sourceFilePath << std::endl;
            return false;
        }

        // Open the target file for writing in binary mode
        std::ofstream targetFile(targetFilePath, std::ios::binary);
        if (!targetFile.is_open())
        {
            std::cerr << "Failed to create target file: " << targetFilePath << std::endl;
            return false;
        }

        // Copy the file contents
        targetFile << sourceFile.rdbuf();

        std::cout << "Audio file copied successfully to: " << targetFilePath << std::endl;
        return true;
    }

    bool AssetManager::UE_DeleteAudioFile(const std::string& filePath)
    {
        if (std::remove(filePath.c_str()) == 0)
        {
            std::cout << "File deleted successfully: " << filePath << std::endl;
            return true;
        }
        else
        {
            std::cerr << "Failed to delete file: " << filePath << std::endl;
            return false;
        }
    }

    void AssetManager::UE_UpdateAudioName(const std::string& oldName, const std::string& newName)
    {
        if (oldName == newName || newName.empty())
        {
            std::cerr << "Invalid or unchanged audio name." << std::endl;
            return;
        }

        auto it = audioAssets.find(oldName);
        if (it == audioAssets.end())
        {
            std::cerr << "AudioAsset not found for name: " << oldName << std::endl;
            return;
        }

        auto& audioAsset = it->second;

        // Extract folder path and file extension
        std::string currentPath = audioAsset.filePath;
        size_t lastSlash = currentPath.find_last_of("/\\");
        std::string folderPath = (lastSlash != std::string::npos) ? currentPath.substr(0, lastSlash + 1) : "";
        size_t dotPos = currentPath.find_last_of(".");
        std::string fileExtension = (dotPos != std::string::npos) ? currentPath.substr(dotPos) : "";

        // Construct new file path
        std::string oldFilePath = folderPath + oldName + fileExtension;
        std::string newFilePath = folderPath + newName + fileExtension;

        // Rename the file on disk
        if (std::rename(oldFilePath.c_str(), newFilePath.c_str()) != 0)
        {
            std::cerr << "Error renaming file from " << oldFilePath << " to " << newFilePath << std::endl;
            return;
        }
        std::cout << "File renamed successfully from " << oldFilePath << " to " << newFilePath << std::endl;

        // Update the MusicAsset with the new file path
        audioAsset.filePath = newFilePath;

        // Ensure the new name does not already exist in the map
        if (audioAssets.find(newName) != audioAssets.end())
        {
            std::cerr << "New name already exists in the map: " << newName << std::endl;
            return;
        }

        // Use nodeHandle to update the map's key efficiently
        auto nodeHandle = audioAssets.extract(it); // Extract the old entry
        nodeHandle.key() = newName;               // Update the key
        audioAssets.insert(std::move(nodeHandle)); // Reinsert with the new key

        // Serialize the updated asset
        AudioAsset::SerializeAudio("Assets/JsonData/AudioAsset.json", audioAssets);
        std::cout << "Audio asset updated and serialized after renaming." << std::endl;
    }

    void AssetManager::UE_DeleteAudio(const std::string& name)
    {
        // Check if the audio asset exists
        auto it = audioAssets.find(name);
        if (it == audioAssets.end())
        {
            std::cerr << "Error: Cannot delete audio. Asset with name '" << name << "' not found." << std::endl;
            return;
        }

        // Get the MusicAsset from the map (no need for a pointer to AudioAsset)
        AudioAsset::MusicAsset& musicAsset = it->second;

        // Get the file path of the audio asset (optional, if you need to delete the file too)
        std::string filePath = musicAsset.filePath;

        // (Optional) Remove the file from disk
        if (std::remove(filePath.c_str()) != 0)
        {
            std::cerr << "Error: Failed to remove file '" << filePath << "' from disk." << std::endl;
        }
        else
        {

            std::cout << "File '" << filePath << "' removed from disk successfully." << std::endl;
        }

        // Remove the asset from the map
        audioAssets.erase(it);
        AudioAsset::SerializeAudio("Assets/JsonData/AudioAsset.json", audioAssets);
        std::cout << "Audio asset '" << name << "' deleted successfully." << std::endl;
    }

    std::vector<std::string> AssetManager::UE_GetAllAudioNames() const
    {
        std::vector<std::string> names;
        for (const auto& pair : audioAssets)
        {
            names.push_back(pair.first);  // The first element of the pair is the custom name (key)
        }
        return names;
    }

    AudioAsset::MusicAsset* AssetManager::UE_GetMusicAssetByName(const std::string& name)
    {
        // Check if the name exists in the audioAssets map
        auto it = audioAssets.find(name);
        if (it != audioAssets.end())
        {
            // Return a pointer to the MusicAsset object found in the map
            return &it->second;
        }
        else
        {
            std::cerr << "Error: MusicAsset with name '" << name << "' not found." << std::endl;
            return nullptr; // Return nullptr if not found
        }
    }

    std::string AssetManager::UE_GetMusicFilePath(const std::string& name)
    {
        // Find the MusicAsset by name in the audioAssets map
        auto it = audioAssets.find(name);
        if (it != audioAssets.end())
        {
            // Return the filePath of the MusicAsset
            return it->second.filePath;
        }
        else
        {
            std::cerr << "Error: MusicAsset with name '" << name << "' not found." << std::endl;
            return ""; // Return empty string if not found
        }
    }

    std::string AssetManager::UE_GetMusicMode(const std::string& name)
    {
        // Find the MusicAsset by name in the audioAssets map
        auto it = audioAssets.find(name);
        if (it != audioAssets.end())
        {
            // Return the mode of the MusicAsset
            return it->second.mode;
        }
        else
        {
            std::cerr << "Error: MusicAsset with name '" << name << "' not found." << std::endl;
            return ""; // Return empty string if not found
        }
    }

    Framework::Audio::SoundType AssetManager::UE_GetMusicSoundType(const std::string& name)
    {
        // Find the MusicAsset by name in the audioAssets map
        auto it = audioAssets.find(name);
        if (it != audioAssets.end())
        {
            // Return the soundType of the MusicAsset
            return it->second.soundType;
        }
        else
        {
            std::cerr << "Error: MusicAsset with name '" << name << "' not found." << std::endl;
            return Framework::Audio::SoundType::EMPTY; // Return default value if not found
        }
    }

    void AssetManager::UE_LoadTexture(const std::string& filePath)
    {
        // Deserialize texture data from the file into the textureAssets map
        TextureAsset::Deserialize(filePath, textureAssets);
        std::cout << "Textures loaded successfully from " << filePath << std::endl;
    }

    TextureAsset::Texture* AssetManager::UE_GetTexture(const std::string& assetName)
    {
        // Check if the texture exists in the map
        auto it = textureAssets.find(assetName);
        if (it != textureAssets.end())
        {
            std::cout << "Texture found: " << assetName << std::endl;
            return &it->second;  // Return pointer to the existing Texture
        }

        // If not found, you can throw an exception or handle the error
        throw std::runtime_error("TextureAsset '" + assetName + "' not found.");
    }

    bool AssetManager::UE_RenameTexture(const std::string& oldName, const std::string& newName)
    {
        // Check if the new name already exists in the map
        if (textureAssets.find(newName) != textureAssets.end())
        {
            std::cerr << "Texture with the name " << newName << " already exists!" << std::endl;
            return false;  // Return false if the new name already exists
        }

        auto it = textureAssets.find(oldName);
        if (it != textureAssets.end())
        {
            // Extract the texture asset and remove the old name
            auto textureAsset = it->second;
            textureAssets.erase(it);

            // Insert the texture with the new name
            textureAssets[newName] = textureAsset;

            std::cout << "Texture renamed from " << oldName << " to " << newName << std::endl;
            return true;  // Successfully renamed
        }
        else
        {
            std::cerr << "Texture with the name " << oldName << " not found!" << std::endl;
            return false;  // Return false if the texture was not found
        }
    }

    void AssetManager::UE_UpdateTextureName(const std::string& oldName, const std::string& newName)
    {
        // Locate the texture in the map
        auto it = textureAssets.find(oldName);
        if (it == textureAssets.end())
        {
            std::cerr << "Error: Texture not found for name " << oldName << std::endl;
            return;
        }

        // Check if the new name already exists in the container
        if (textureAssets.find(newName) != textureAssets.end())
        {
            std::cerr << "Error: A texture with the name \"" << newName << "\" already exists." << std::endl;
            return;
        }

        // Access the texture object
        TextureAsset::Texture& texture = it->second;

        // Extract the directory and file extension
        std::string oldPath = texture.path; // Existing file path
        size_t pos = oldPath.find_last_of("/\\"); // Last separator (platform-agnostic)
        std::string pathWithoutName = (pos != std::string::npos) ? oldPath.substr(0, pos + 1) : ""; // Directory path
        std::string fileExtension = oldPath.substr(oldPath.find_last_of(".")); // File extension (e.g., .png)

        // Construct the new path
        std::string newPath = pathWithoutName + newName + fileExtension;

        // Rename the file on disk
        if (rename(oldPath.c_str(), newPath.c_str()) != 0)
        {
            std::perror("Failed to rename file");
            return;
        }
        std::cout << "File renamed successfully: " << oldPath << " to " << newPath << std::endl;

        // Update the texture object
        texture.name = newName;  // Update name in the texture data
        texture.path = newPath;  // Update path in the texture data

        // Remove the old key and add the updated key
        textureAssets[newName] = std::move(texture); // Move the texture to the new key
        textureAssets.erase(it); // Erase the old key

        std::cout << "Texture name updated in the container: " << oldName << " to " << newName << std::endl;
        TextureAsset::Serialize("Assets/JsonData/TextureAsset.json", textureAssets);
    }

    bool CopyTextureToFolder(const std::string& sourceFilePath, const std::string& targetFolder)
    {
        // Extract the filename from the source path
        size_t pos = sourceFilePath.find_last_of("/\\");
        std::string fileName = sourceFilePath.substr(pos + 1);

        // Construct the target file path
        std::string targetFilePath = targetFolder + "\\" + fileName;

        // Open the source file for reading in binary mode
        std::ifstream sourceFile(sourceFilePath, std::ios::binary);
        if (!sourceFile.is_open())
        {
            std::cerr << "Failed to open source file: " << sourceFilePath << std::endl;
            return false;
        }

        // Open the target file for writing in binary mode
        std::ofstream targetFile(targetFilePath, std::ios::binary);
        if (!targetFile.is_open())
        {
            std::cerr << "Failed to create target file: " << targetFilePath << std::endl;
            return false;
        }

        // Copy the file contents
        targetFile << sourceFile.rdbuf();

        std::cout << "File copied successfully to: " << targetFilePath << std::endl;
        return true;
    }

    void AssetManager::UE_AddTexture(const std::string& name, const std::string& path)
    {
        // Define the base folder for textures
        std::string targetFolder = "Assets/Images";  // Fixed folder for storing textures

        // Extract the file name from the source path
        size_t pos = path.find_last_of("/\\");
        std::string fileName = path.substr(pos + 1);  // Get only the file name
        std::string targetPath = targetFolder + "/" + fileName;  // Full path in the target folder

        // Copy the texture to the target folder
        if (!CopyTextureToFolder(path, targetFolder))
        {
            std::cerr << "Failed to copy texture to target folder." << std::endl;
            return;
        }
        std::cout << "Texture successfully copied to target folder." << std::endl;

        // Check if a Texture already exists for this name
        auto it = textureAssets.find(name);
        if (it == textureAssets.end())
        {
            std::cout << "No existing Texture found for name: " << name << ". Creating a new one." << std::endl;

            // Create a new Texture and add it to the map
            TextureAsset::Texture newTexture;
            newTexture.name = name;
            newTexture.path = targetPath;

            // Insert the new Texture into the map
            textureAssets[name] = newTexture;
            std::cout << "Texture added successfully." << std::endl;

            // Optionally, serialize the updated texture map (not a single asset, but the full set)
            TextureAsset::Serialize("Assets/JsonData/TextureAsset.json", textureAssets);
        }
        else
        {
            std::cout << "Found existing Texture for name: " << name << ". Updating the path." << std::endl;

            // If the Texture exists, just update its path
            it->second.path = targetPath;  // Update the path for the existing texture

            // Serialize the updated texture map (not a single asset, but the full set)
            TextureAsset::Serialize("Assets/JsonData/TextureAsset.json", textureAssets);

            std::cout << "Texture path updated successfully." << std::endl;
        }
    }

    void AssetManager::UE_DeleteTexture(const std::string& textureName)
    {
        // Find the texture in the unordered map
        auto it = textureAssets.find(textureName);
        if (it != textureAssets.end())
        {
            // Get the file path associated with the texture
            std::string filePath = it->second.path; // Assume this function exists in your TextureAsset

            // Remove the texture from the unordered_map
            textureAssets.erase(it);

            // Attempt to delete the file from the folder
            if (std::remove(filePath.c_str()) == 0)
            {
                std::cout << "File " << filePath << " deleted successfully." << std::endl;
            }
            else
            {
                std::cerr << "Failed to delete file " << filePath << "! Please check permissions or path." << std::endl;
            }

            // Re-serialize the entire set of textures
            TextureAsset::Serialize("Assets/JsonData/TextureAsset.json", textureAssets);

            std::cout << "Texture " << textureName << " deleted." << std::endl;
        }
        else
        {
            std::cerr << "Texture " << textureName << " not found!" << std::endl;
        }
    }

    std::string AssetManager::UE_GetTexturePath(const std::string& textureName)
    {
        // Find the texture in the unordered map
        auto it = textureAssets.find(textureName);
        if (it != textureAssets.end())
        {
            // Return the path of the texture from the Texture struct
            return it->second.path;
        }
        else
        {
            std::cerr << "Texture " << textureName << " not found!" << std::endl;
            return "";
        }
    }

    GLuint AssetManager::UE_LoadTextureToOpenGL(const std::string& textureName)
    {
        // Find the texture in the textureAssets map
        auto it = textureAssets.find(textureName);
        if (it == textureAssets.end())
        {
            std::cerr << "Texture with name '" << textureName << "' not found!" << std::endl;
            return 0;  // Return 0 if the texture is not found
        }

        // Retrieve the texture path from the found texture
        const std::string& textureFilePath = it->second.path;

        // Check if the texture has already been loaded (has a textureID)
        if (it->second.textureID != 0)
        {
            return it->second.textureID;  // Return the existing textureID
        }

        // Use stb_image to load the texture from file
        int width, height, nrChannels;
        data = stbi_load(textureFilePath.c_str(), &width, &height, &nrChannels, 0);
        if (!data)
        {
            //std::cerr << "Failed to load texture at path: " << textureFilePath << std::endl;
            return 0;  // Return 0 if loading fails
        }

        // If not loaded, generate a new texture ID and load the texture from the file
        GLuint textureID;
        glGenTextures(1, &textureID);
        if (textureID == 0)
        {
            std::cerr << "Failed to generate texture ID" << std::endl;
            stbi_image_free(data);
            return 0;
        }
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Determine texture format based on channels
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

        // Generate the texture and load the image into OpenGL
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Free image memory
        stbi_image_free(data);

        // Store the generated textureID in the texture map for future use
        it->second.textureID = textureID;  // Store the textureID in the Texture object

        //std::cout << "Loaded texture with name '" << textureName << "' and ID: " << textureID << std::endl;

        return textureID;
    }

    std::string AssetManager::UE_LoadGraphicsShader(const std::string& filePath)
    {
        // Check if the shader is already loaded
        if (graphicShaderSources.find(filePath) != graphicShaderSources.end())
        {
            return graphicShaderSources[filePath]; // Shader already loaded, no need to load again
        }

        std::ifstream shaderFile(filePath);
        if (!shaderFile.is_open())
        {
            throw std::runtime_error("Failed to open shader file: " + filePath);
        }

        std::ostringstream shaderStream;
        shaderStream << shaderFile.rdbuf(); // Read the file buffer into the stream
        graphicShaderSources[filePath] = shaderStream.str(); // Store the shader code in the container
        return shaderStream.str();
    }

    const std::string& AssetManager::UE_GetShaderSource(const std::string& shaderKey) const
    {
        auto it = graphicShaderSources.find(shaderKey);
        if (it != graphicShaderSources.end())
        {
            return it->second; // Return the shader source string
        }
        else
        {
            // Handle case where the key is not found
            throw std::runtime_error("Shader key not found: " + shaderKey);
        }
    }

    std::string AssetManager::UE_LoadFontShader(const std::string& filePath)
    {
        // Check if the shader is already loaded
        if (fontShaderSources.find(filePath) != fontShaderSources.end()) 
        {
            return fontShaderSources[filePath]; // Shader already loaded, return it
        }

        // Try to open the shader file
        std::ifstream shaderFile(filePath);
        if (!shaderFile.is_open()) 
        {
            std::cerr << "Failed to open shader file: " << filePath << std::endl;
            return ""; // Return empty string if the file can't be opened
        }

        // Read the content of the file into a stringstream
        std::stringstream buffer;
        buffer << shaderFile.rdbuf();
        std::string shaderCode = buffer.str();

        // Store the shader code in the container (fontShaderSources)
        fontShaderSources[filePath] = shaderCode;

        return shaderCode; // Return the shader code
    }

    void AssetManager::UE_GetFontShader(const std::string& assetName)
    {
        (void)assetName;
    }

    bool AssetManager::UE_LoadFont(const std::string& fontPath, int fontSize, const std::string& fontName) 
    {
        FT_Library& ftLib = fontSystem.GetFTLibrary();      // Font System FT Library

        if (fontCacheAssets.find(fontName) != fontCacheAssets.end()) {
            std::cerr << "Font " << fontName << " is already loaded. Skipping reload." << std::endl;
            return true;
        }

        FT_Face face;
        if (FT_New_Face(ftLib, fontPath.c_str(), 0, &face)) 
        {
            std::cerr << "Failed to load font: " << fontPath << std::endl;
            return false;
        }
        else
        {
            std::cout << "Successfully loaded font: " << fontPath << std::endl;
        }

        FT_Set_Pixel_Sizes(face, 0, fontSize);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

        std::unordered_map<char, Character> characters;

        for (unsigned char c = 0; c < 128; c++) {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                std::cerr << "Failed to load Glyph: " << c << std::endl;
                continue;
            }

            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            Character character = {
                texture,
                { face->glyph->bitmap.width, face->glyph->bitmap.rows },
                { face->glyph->bitmap_left, face->glyph->bitmap_top },
                static_cast<GLuint>(face->glyph->advance.x)
            };

            characters[c] = character; // Store character in the map
        }

        fontCacheAssets[fontName] = characters; 
        FT_Done_Face(face); // Frees face resources
        std::cout << "Font " << fontName << " loaded successfully." << std::endl;
        std::cout << "Current font assets: " << fontCacheAssets.size() << std::endl;
        return true;
    }
}