///////////////////////////////////////////////////////////////////////////////
///
///	@File  : WindowAsset.cpp
/// @Brief : Implements the `Window` class, which handles the configuration of
///          application window settings such as width, height, and title.
///          Provides functionality for deserializing window configuration from
///          a JSON file and initializing the `Window` object based on this data.
///          The file also includes logic to parse a JSON file with specific
///          structure and retrieve window settings for the application.
///
///	@Main Author : Edwin Leow (100%)
/// @Secondary Author : NIL
///	@Copyright 2024, Digipen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "WindowAsset.h"

/**
 * @brief Constructs a Window object and loads window configuration from the specified file.
 * @param filePath Path to the JSON file containing window configuration.
 */
Window::Window(const std::string& filePath)
{
    Deserialize(filePath);
}

/**
 * @brief Destructor for the Window class. Currently, no dynamic memory is allocated,
 *        so the destructor is empty.
 */
Window::~Window() {}

/**
 * @brief Deserializes the window configuration from a JSON file.
 *        The function parses the JSON, checks for a specific structure, and
 *        extracts the window properties (width, height, and title).
 * @param filePath Path to the JSON file containing window configuration.
 */
void Window::Deserialize(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return;
    }

    rapidjson::IStreamWrapper isw(file);    // Wrap the input file stream
    rapidjson::Document document;           // Parse the JSON document
    document.ParseStream(isw);

    // Check if the "windows" key exists and is an array
    if (document.HasMember("windows") && document["windows"].IsArray()) 
    {
        const rapidjson::Value& windowsArray = document["windows"];

        // Extract the first object in the array
        if (windowsArray.Size() > 0 && windowsArray[0].IsObject()) 
        {
            const rapidjson::Value& windowObject = windowsArray[0];

            // Extract x, y, and program_name values
            if (windowObject.HasMember("x") && windowObject["x"].IsInt()) 
            {
                windowConfig.x = windowObject["x"].GetInt();
            }

            if (windowObject.HasMember("y") && windowObject["y"].IsInt()) 
            {
                windowConfig.y = windowObject["y"].GetInt();
            }

            if (windowObject.HasMember("program_name") && windowObject["program_name"].IsString()) 
            {
                windowConfig.programName = windowObject["program_name"].GetString();
            }

            // Output or store window configuration
            //std::cout << "Window X: " << windowConfig.x << "\n";
            //std::cout << "Window Y: " << windowConfig.y << "\n";
            //std::cout << "Program Name: " << windowConfig.programName << "\n";
        }
    }
    else 
    {
        std::cerr << "The 'windows' key is missing or not an array!\n";
    }
}