/*
Copyright (c) Anthony Beaumont
This source code is licensed under the MIT License
found in the LICENSE file in the root directory of this source tree.
*/

#pragma once
#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include <fstream>
#include <filesystem>
#include <string>
#include "../vendor/nlohmann/json.hpp"

struct Patch {
    std::string name        = {};
    std::string pattern     = {};
    std::string value       = {};
    std::string match       = {};
    std::string module      = {};
    int offset              = 0;
    bool enable             = false;
};

void from_json(const nlohmann::json& json, Patch& patch);
DWORD WINAPI Main(LPVOID lpReserved);
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved);