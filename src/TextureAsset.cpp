///////////////////////////////////////////////////////////////////////////////
///
///	@File  : TextureAsset.cpp
/// @Brief : Defines the TextureAsset class, responsible for managing texture
///          assets in the application. This includes loading textures from files,
///          serializing/deserializing texture data, and providing access to texture
///          properties such as names, paths, and OpenGL texture IDs.
///          The class supports functionalities like loading textures from a JSON
///          file, serializing them back to a file, and managing texture-related
///          information dynamically.
///
///	@Main Author : Edwin Leow (100%)
/// @Secondary Author : NIL
///	@Copyright 2024, Digipen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "TextureAsset.h"
#include "AssetManager.h"

void TextureAsset::Deserialize(const std::string& filePath, std::unordered_map<std::string, TextureAsset::Texture>& imageAssets)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    rapidjson::Document document;
    document.Parse(buffer.str().c_str());

    if (!document.IsObject())
    {
        std::cerr << "Invalid JSON format." << std::endl;
        return;
    }

    const auto& textures = document["textures"];
    if (!textures.IsArray())
    {
        std::cerr << "Textures field is not an array." << std::endl;
        return;
    }

    for (const auto& texture : textures.GetArray())
    {
        if (texture.IsObject())
        {
            TextureAsset::Texture newTexture;

            // Deserialize texture name
            if (texture.HasMember("name") && texture["name"].IsString())
            {
                newTexture.name = texture["name"].GetString();
            }

            // Deserialize texture path
            if (texture.HasMember("path") && texture["path"].IsString())
            {
                newTexture.path = texture["path"].GetString();
            }

            // Add the texture to the provided imageAssets map
            imageAssets[newTexture.name] = newTexture;
        }
    }
}

void TextureAsset::Serialize(const std::string& filePath, const std::unordered_map<std::string, TextureAsset::Texture>& imageAssets)
{
    rapidjson::Document document;

    // Open file for reading (if it exists) and parse it
    std::ifstream inFile(filePath);
    if (inFile.is_open())
    {
        std::string fileContents((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
        document.Parse(fileContents.c_str());

        // Check for any parse errors
        if (document.HasParseError()) 
        {
            std::cerr << "Failed to parse JSON file." << std::endl;
            return;
        }

        // Check if the 'textures' key exists and is an array, if not, initialize it as an array
        if (!document.HasMember("textures") || !document["textures"].IsArray()) 
        {
            rapidjson::Value texturesArray(rapidjson::kArrayType);
            document.AddMember("textures", texturesArray, document.GetAllocator());  // Ensure we add an empty array if it's missing
        }
    }
    else
    {
        // If file does not exist, initialize document and add empty textures array
        document.SetObject();
        rapidjson::Value texturesArray(rapidjson::kArrayType);
        document.AddMember("textures", texturesArray, document.GetAllocator());
    }

    // Now we ensure that texturesArray is accessible
    rapidjson::Value& texturesArray = document["textures"];

    // Clear the textures array first, so we don't retain deleted textures
    texturesArray.Clear();

    // Iterate over the imageAssets map and add each texture to the texturesArray
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    // Iterate over each texture in the provided imageAssets map
    for (const auto& texturePair : imageAssets)
    {
        const TextureAsset::Texture& texture = texturePair.second;

        // Create a JSON object for this texture
        rapidjson::Value textureObject(rapidjson::kObjectType);
        textureObject.AddMember("name", rapidjson::Value(texture.name.c_str(), allocator), allocator);
        textureObject.AddMember("path", rapidjson::Value(texture.path.c_str(), allocator), allocator);

        // Add this texture to the textures array
        texturesArray.PushBack(textureObject, allocator);
    }

    // Write the updated document to the file
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    std::ofstream outFile(filePath);
    if (outFile.is_open())
    {
        outFile << buffer.GetString();  // Write the JSON string to the file
        outFile.close();
        std::cout << "Successfully serialized textures to " << filePath << std::endl;
    }
    else
    {
        std::cerr << "Failed to open file for writing: " << filePath << std::endl;
    }
}
