///////////////////////////////////////////////////////////////////////////////
///
///	@File  : WindowAsset.h
/// @Brief : Declares the Window class, which handles the configuration of
///          application window settings, including width, height, and title.
///          This class provides deserialization functionality for loading 
///          configuration from JSON files.
///
///	@Main Author : Edwin Leow (100%)
/// @Secondary Author : NIL
///	@Copyright 2024, Digipen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef _WINDOW_H_
#define _WINDOW_H_
#include "pch.h"
#include "JsonSerialize.h"

/**
 * @class Window
 * @brief Manages window configurations, including deserializing from JSON files.
 */
class Window
{
public:

    /**
     * @brief Constructs a Window object and loads configuration from a file.
     * @param filePath The path to the configuration file.
     */
	Window(const std::string& filePath);
    
    /**
     * @brief Destructor for the Window class.
     */
    ~Window();
	
    /**
     * @brief Deserializes window configuration from a JSON file.
     * @param filePath Path to the JSON configuration file.
     */
    void Deserialize(const std::string& filePath);  // Function to deserialize from Json
	
    /**
        *   @struct WindowConfig
        *   @brief Struct representing the window configuration for the application.
        */
    struct WindowConfig
    {
        int x;                      // Width of the window
        int y;                      // Height of the window
        std::string programName;    // Name of the program/Title

        /**
         * @brief Default constructor initializing WindowConfig with default values.
         */
        WindowConfig() : x(0), y(0), programName("") {}
    };

    /**
     * @brief Retrieves the window configuration.
     * @return A constant reference to the WindowConfig object.
     */
    const WindowConfig& GetConfig() const
    {
        return windowConfig;                        // Return private data member
    }

private:
    WindowConfig windowConfig;  // Stores the configuration for the window
};

#endif // !_WINDOW_H_