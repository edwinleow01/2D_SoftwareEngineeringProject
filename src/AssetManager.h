///////////////////////////////////////////////////////////////////////////////
///
///	@File  : AssetManager.h
/// @Brief : This file implements the AssetManager class, a central component 
///          responsible for managing various asset types within the application, 
///          including audio, textures, fonts, and shaders. The AssetManager 
///          provides methods to load and retrieve these assets, allowing for 
///          efficient reuse across different systems.
///
///	@Main Author : Edwin Leow (100%)
///	@Secondary Author : NIL
/// @Copyright 2024, Digipen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef _ASSET_MANAGER_H_
#define _ASSET_MANAGER_H_
#include "pch.h"
#include "unordered_map"

#include <glm.hpp>
#include <glew.h>
#include <gtc/matrix_transform.hpp>
#include <ft2build.h>

// Graphics Header Files
#include "stb_image.h"
#include "stb_image_resize2.h"

// Sub Asset Header Files
#include "WindowAsset.h"
#include "EntityAsset.h"
#include "AudioAsset.h"
#include "TextureAsset.h"
#include "lexicon.h"

// Forward declaration of asset types here
class Window;
class EntityAsset;
class AudioAsset;
class TextureAsset;

namespace Framework
{
    /**
     * @class AssetManager
     * @brief Manages various asset types, including window configurations, dictionaries,
     *        ECS entities, audio, textures, shaders, and fonts. Provides loading and
     *        retrieval functions for these assets.
     */
    class AssetManager
    {
    public:
        /**
         * @brief Default constructor for AssetManager.
         */
        AssetManager();

        /**
         * @brief Destructor for AssetManager. Cleans up managed assets.
         */
        ~AssetManager()
        {
            std::cout << "Destructor assetmanager called" << std::endl;
            fontCacheAssets.clear();
            fontShaderSources.clear();
            graphicShaderSources.clear();
            textureAssets.clear();
            audioAssets.clear();
            entityAssets.clear();
            windowAssets.clear();
            nsfwList.clear();
            dictionaryWords.clear();
            prefixList.clear();
            bulletDataMap.clear();
            animationDataMap.clear();
            // delete data; // Removed
        }

        /***********************/
        //   Window Functions  //
        /***********************/

        /**
         * @brief Loads a window configuration from a specified file path.
         * @param filePath Path to the window configuration file.
         * @return A reference to the loaded Window object.
         */
        Window& UE_LoadWindow(const std::string& filePath);	                        // Load a window from a filepath
        
        /************************/
        //   Dictionary/Prefix  //
        /************************/

        /**
         * @brief Loads dictionary data from a specified file.
         * @param fileName Path to the dictionary file.
         */
        void UE_LoadDictionary(const std::string& fileName);       

        /**
         * @brief Loads prefix data from a specified file.
         * @param fileName Path to the prefix file.
         */
        void UE_LoadPrefixes(const std::string& fileName);

        /**
         * @brief Loads NSFW-related data from a specified file into the NSFW list.
         * @param fileName Path to the NSFW data file.
         */
        void UE_LoadNSFW(const std::string& fileName);

        /**
         * @brief Retrieves the list of NSFW assets.
         * @return A constant reference to the vector containing NSFW-related asset names.
         */
        const std::vector<std::string>& GetNSFWAssets() const { return nsfwList; }

        /**
         * @brief Retrieves the list of dictionary words.
         * @return A constant reference to the vector containing dictionary words.
         */
        const std::vector<std::string>& GetDictionaryAssets() const { return dictionaryWords; }

        /**
         * @brief Retrieves the list of prefix assets.
         * @return A constant reference to the vector containing prefix words.
         */
        const std::vector<std::string>& GetPrefixAssets() const { return prefixList; }

        /***************/
        //   Entities  //
        /***************/

        /**
         * @brief Loads ECS entities from a specified file path.
         * @param filePath Path to the file containing ECS entity data.
         * @return A reference to the loaded EntityAsset object.
         */
        void UE_LoadEntities(const std::string& filePath);                     // Load ECS Objects

        /**
         * @brief Loads ECS entities from a specified file in nested prefab folder.
         * @param filePath Path to the file containing ECS entity data.
         * @return A reference to the loaded EntityAsset object.
         */
        void UE_LoadPrefab(const std::string& filePath, glm::vec2 Location = glm::vec2(-1,-1));                     // Load ECS Objects

        /**
         * @brief Retrieves all loaded ECS entities.
         * @return A reference to an unordered map containing all EntityAssets.
         */
        std::unordered_map<std::string, std::unique_ptr<EntityAsset>>& UE_GetAllEntities() { return entityAssets; }

        /**********************/
        //   Audio Functions  //
        /**********************/

        /**
         * @brief Loads an audio asset from a specified file path.
         * @param filePath Path to the audio file.
         * @return A shared pointer to the loaded AudioAsset object.
         */
        void UE_LoadAudio(const std::string& filePath);         // Load Audio
        
        /**
         * @brief Retrieves a loaded audio asset by name.
         * @param assetName Name of the audio asset.
         * @return A pointer to the AudioAsset object.
         */
        AudioAsset::MusicAsset* UE_GetAudioAsset(const std::string& assetName);                    // Method to retrieve an audio asset

        /**
         * @brief Retrieves all loaded audio assets.
         * @return A constant reference to an unordered map of audio assets.
         */
        const std::unordered_map<std::string, AudioAsset::MusicAsset>& UE_GetAllAudioAssets() const { return audioAssets; }

        /**
         * @brief Adds a new audio asset to the manager from the given path.
         * @param path Path to the audio file.
         */
        void UE_AddAudio(const std::string& path);

        /**
         * @brief Copies an audio file to a specified folder.
         * @param sourceFilePath Path to the source audio file.
         * @param targetFolder Target folder where the file should be copied.
         * @return True if the operation was successful, false otherwise.
         */
        bool UE_CopyAudioToFolder(const std::string& sourceFilePath, const std::string& targetFolder);

        /**
         * @brief Deletes an audio file from disk.
         * @param filePath Path to the audio file.
         * @return True if the file was successfully deleted, false otherwise.
         */
        bool UE_DeleteAudioFile(const std::string& filePath);

        /**
         * @brief Updates the name of an existing audio asset.
         * @param oldName Current name of the audio asset.
         * @param newName New name to assign to the audio asset.
         */
        void UE_UpdateAudioName(const std::string& oldName, const std::string& newName);
        
        /**
         * @brief Deletes an audio asset from the manager.
         * @param name Name of the audio asset to delete.
         */
        void UE_DeleteAudio(const std::string& name);

        /**
         * @brief Retrieves the names of all loaded audio assets.
         * @return A vector containing the names of all audio assets.
         */
        std::vector<std::string> UE_GetAllAudioNames() const;

        /**
         * @brief Retrieves a music asset by its name.
         * @param name Name of the music asset.
         * @return A pointer to the MusicAsset if found, nullptr otherwise.
         */
        AudioAsset::MusicAsset* UE_GetMusicAssetByName(const std::string& name);

        /**
         * @brief Retrieves the file path of a music asset by its name.
         * @param name Name of the music asset.
         * @return The file path of the music asset as a string.
         */
        std::string UE_GetMusicFilePath(const std::string& name);

        /**
         * @brief Retrieves the mode of a music asset by its name.
         * @param name Name of the music asset.
         * @return The mode of the music asset as a string.
         */
        std::string UE_GetMusicMode(const std::string& name);

        /**
         * @brief Retrieves the sound type of a music asset by its name.
         * @param name Name of the music asset.
         * @return The sound type of the music asset.
         */
        Framework::Audio::SoundType UE_GetMusicSoundType(const std::string& name);

        const std::unordered_map<std::string, AudioAsset::MusicAsset>& GetMusicAssets() { return audioAssets; }

        /************************/
        //   Texture Functions  //
        /************************/

        /**
         * @brief Loads a texture asset from a specified file path.
         * @param filePath Path to the texture file.
         * @return A shared pointer to the loaded TextureAsset object.
         */
        void UE_LoadTexture(const std::string& filePath);     // Load Texture
        
        /**
         * @brief Retrieves a loaded texture asset by name.
         * @param assetName Name of the texture asset.
         * @return A pointer to the TextureAsset object.
         */
        TextureAsset::Texture* UE_GetTexture(const std::string& assetName);                     // Get texture based on name

        /**
         * @brief Retrieves all loaded texture assets.
         * @return A reference to an unordered map of texture assets.
         */
        std::unordered_map<std::string, TextureAsset::Texture>& UE_GetAllTextureAssets() { return textureAssets; }

        /**
         * @brief Renames an existing texture asset.
         * @param oldName Current name of the texture.
         * @param newName New name to assign to the texture.
         * @return True if the renaming was successful, false otherwise.
         */
        bool UE_RenameTexture(const std::string& oldName, const std::string& newName);

        /**
         * @brief Updates the name of an existing texture asset.
         * @param oldName Current name of the texture.
         * @param newName New name to assign to the texture.
         */
        void UE_UpdateTextureName(const std::string& oldName, const std::string& newName);

        /**
         * @brief Deletes a texture asset from the manager.
         * @param textureName Name of the texture asset to delete.
         */
        void UE_DeleteTexture(const std::string& textureName);

        /**
         * @brief Retrieves the file path of a texture asset by name.
         * @param textureName Name of the texture asset.
         * @return The file path of the texture asset as a string.
         */
        std::string UE_GetTexturePath(const std::string& textureName);

        /**
         * @brief Loads a texture into OpenGL and returns its ID.
         * @param textureName Name of the texture asset.
         * @return The OpenGL texture ID.
         */
        GLuint UE_LoadTextureToOpenGL(const std::string& textureName);

        /**
         * @brief Adds a texture to the manager from a specified name and path.
         * @param name The name associated with the texture.
         * @param path The file path to the texture.
         */
        void UE_AddTexture(const std::string& name, const std::string& path);

        /********************************/
        //   Graphics Shader Functions  //
        /********************************/

        /**
         * @brief Loads and stores a shader source from a specified file path.
         * @param filePath Path to the shader file.
         * @return A string containing the shader source code.
         */
        std::string UE_LoadGraphicsShader(const std::string& filePath);
        
        /**
         * @brief Retrieves a shader source code by its key.
         * @param shaderKey Key identifying the shader.
         * @return A constant reference to the shader source code string.
         */
        const std::string& UE_GetShaderSource(const std::string& shaderKey) const;

        /*********************/
        //   Font Functions  //
        /*********************/
        
        /**
         * @brief Represents a single character in a loaded font.
         *
         * This struct stores all the necessary information for a specific character in a font, such as its texture,
         * size, bearing (offset from the baseline), and advance (spacing between characters). It is used to render
         * individual characters when displaying text on the screen.
         */
        struct Character
        {
            GLuint TextureID;
            glm::ivec2 Size;
            glm::ivec2 Bearing;
            GLuint Advance;
        };

        /**
         * @brief Loads a font asset from a specified file path and size.
         * @param filePath Path to the font file.
         * @param fontSize Size of the font to be loaded.
         * @return A pointer to the loaded FontAsset object.
         */
        bool UE_LoadFont(const std::string& filePath, int fontSize, const std::string& fontName);             // Load Font
        
        /**
         * @brief Retrieves the font cache assets.
         * @return A constant reference to a map of font names to their corresponding character maps.
         */
        const std::unordered_map<std::string, std::unordered_map<char, Character>>& GetFontCacheAssets() const { return fontCacheAssets; }

        /****************************/
        //   Font Shader Functions  //
        /****************************/

        /**
         * @brief Loads a font shader from a specified file path.
         * @param filePath Path to the shader file.
         * @return A string containing the shader source code.
         */
        std::string UE_LoadFontShader(const std::string& filePath);         // Load Font Shader
        
        /**
         * @brief Retrieves a loaded font shader by name.
         * @param assetName Name of the font shader asset.
         * @return A pointer to the FontShaderAsset object.
         */
        void UE_GetFontShader(const std::string& assetName);               // Get Font Shader based on name

        void StoreBulletData(const std::string& name, const EntityAsset::BulletData& bulletData)
        {
            bulletDataMap[name] = bulletData;
        }

        const EntityAsset::BulletData* GetBulletData(const std::string& name) const
        {
            auto it = bulletDataMap.find(name);
            return (it != bulletDataMap.end()) ? &it->second : nullptr;
        }

        // Getter function to access the entire animationDataMap
        std::unordered_map<std::string, EntityAsset::Animation>& GetAnimationDataMap()
        {
            return animationDataMap;
        }

        static unsigned char* data;     // Static data buffer used for image loading

    private:
        std::unordered_map<std::string, std::unique_ptr<Window>> windowAssets;                          // Container for Windowconfig
        std::vector<std::string> dictionaryWords;
        std::vector<std::string> prefixList;
        std::vector<std::string> nsfwList;
        std::unordered_map<std::string, std::unique_ptr<EntityAsset>> entityAssets;                     // Container for EntityAsset
        std::unordered_map<std::string, AudioAsset::MusicAsset> audioAssets;                            // Container for AudioAsset
        std::unordered_map<std::string, TextureAsset::Texture> textureAssets;                           // Container for TextureAsset
        std::unordered_map<std::string, std::string> graphicShaderSources;                              // Container for Graphics Shader
        std::unordered_map<std::string, std::unordered_map<char, Character>> fontCacheAssets;           // Container for Font Assets
        std::unordered_map<std::string, std::string> fontShaderSources;                                 // Container for Font Shader
        std::unordered_map<std::string, EntityAsset::BulletData> bulletDataMap;                         // Container for Bullet Data
        std::unordered_map<std::string, EntityAsset::Animation> animationDataMap;
    };
    extern AssetManager GlobalAssetManager;  // Global instance of AssetManager, defined in AssetManager.cpp
}
#endif // !_ASSET_MANAGER_H_