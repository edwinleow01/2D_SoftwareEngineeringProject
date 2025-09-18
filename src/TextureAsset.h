///////////////////////////////////////////////////////////////////////////////
///
///	@File  : TextureAsset.h
/// @Brief : Defines the TextureAsset class, responsible for managing texture
///          assets in the application. This includes loading textures from files,
///          serializing/deserializing texture data, and providing access to texture
///          properties such as names, paths, and OpenGL texture IDs.
///
///	@Main Author : Edwin Leow (100%)
/// @Secondary Author : NIL
///	@Copyright 2024, Digipen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef _TEXTURE_ASSET_H_
#define _TEXTURE_ASSET_H_
#include "pch.h"
#include "JsonSerialize.h"
#include <glew.h>

/**
 * @class TextureAsset
 * @brief Manages texture assets, including loading, serializing, and providing access
 *        to texture properties such as names, paths, and OpenGL texture IDs.
 */
class TextureAsset
{
public:

	/**
	 * @struct Texture
	 * @brief Represents a single texture with a name and path.
	 */
	struct Texture
	{
		std::string name;
		std::string path;
		GLuint textureID = 0;  // Store texture ID once loaded
	};

	/**
	 * @brief Default constructor for TextureAsset.
	 */
	TextureAsset() = default;
	
	/**
	 * @brief Constructs a TextureAsset and loads texture data from a file.
	 * @param filePath Path to the texture configuration file.
	 */
	TextureAsset(const std::string& filePath) : filePath(filePath) {}

	/**
	 * @brief Destructor for the TextureAsset class.
	 */
	~TextureAsset() = default;

	/**
	 * @brief Deserializes texture data from a file.
	 * @param filePath Path to the texture configuration file.
	 */
	static void Deserialize(const std::string& filePath, std::unordered_map<std::string, TextureAsset::Texture>& imageAssets);

	/**
	 * @brief Serializes texture data to a file.
	 * @param filePath Path where the texture data will be saved.
	 */
	static void Serialize(const std::string& filePath, const std::unordered_map<std::string, TextureAsset::Texture>& imageAssets);

private:
	std::string filePath;

};
#endif // !_TEXTURE_ASSET_H_