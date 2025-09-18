///////////////////////////////////////////////////////////////////////////////
///
/// @File   : JsonSerialize.h
/// @Brief  : Includes all necessary files for Rapidjson and Standard Library 
///			  for storing data in containers.
///
/// @Main Author : Edwin Leow (100%)
/// @Secondary Author : NIL
/// @Copyright 2024, Digipen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef _JSONSERIALIZE_H_
#define _JSONSERIALIZE_H_

// RapidJson
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/error/en.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/allocators.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

// Standard Library
#include "fstream"
#include "sstream"
#include "unordered_map"
#include "unordered_set"

#endif // _JSONSERIALIZE_H_