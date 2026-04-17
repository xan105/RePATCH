/*
Copyright (c) Anthony Beaumont
This source code is licensed under the MIT License
found in the LICENSE file in the root directory of this source tree.
*/

#include "dllmain.h"
#include "version.h"
#include "memory.h"
#include "./util/logger.hpp"
#include "./util/win32.h"

void from_json(const nlohmann::json& json, Patch& patch) {
    patch.enable      = json.value("enable", true);
    patch.name        = json.value("name", "");
    patch.pattern     = json.value("pattern", "");
    patch.value       = json.value("value", "");
    patch.module      = json.value("module", "");

    std::vector<std::string> match_values = {"first", "last", "all"};
    patch.match = json.value("match", match_values.at(0));
    if (std::find(match_values.begin(), match_values.end(), patch.match) == match_values.end()) {
        patch.match = match_values.at(0);
    }
    
    patch.offset = 0;
    if (json.contains("offset")) {
        const auto& offset = json["offset"];
        if (offset.is_number_integer()) {
            patch.offset = offset.get<int>();
        }
        else if (offset.is_string()) {
            std::string hex = offset.get<std::string>();
            if (hex.starts_with("0x") || hex.starts_with("0X")) {
                patch.offset = std::stoi(hex, nullptr, 16);
            }
        }
    }
}

DWORD WINAPI Main(LPVOID lpReserved) {
    #ifdef _DEBUG
    enableConsole();
    Logger console("RePATCH.log");
    console.setLogLevel(Logger::Level::info);
    #else
    Logger console;
    console.setLogLevel(Logger::Level::error);
    #endif
    try {
        console.log("RePATCH version: {}", VER_FILEVERSION_STR);
      
        std::filesystem::path filepath = Getenv(L"REPATCH_FILEPATH");
        if (filepath.empty()) filepath = L"patch.json";
        console.log("Patch file: {}", filepath.string());

        std::ifstream file{filepath};
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open: " + filepath.string());
        }

        nlohmann::json json;
        file >> json;

        std::vector<Patch> patches = json.get<std::vector<Patch>>();
        for (const auto& patch : patches) {
            if (!patch.enable) continue;
            console.log("Patch: \"{}\"", patch.name);

            auto addresses = memory::find(patch.module, patch.pattern, patch.match);
            if (addresses.empty()) {
                console.error("No pattern found!");
            }

            for (const auto& address : addresses) {
                console.log("Found pattern at 0x{:X}", address);
                if (memory::write(address, patch.offset, patch.value)) {
                    console.log("Applying patch... Succes!");
                } else {
                    console.error("Applying patch... Failed!");
                }
            }
        }
    } catch (const std::exception& error) {
        console.error("{}", error.what());
        return 1;
    }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH: {
            DisableThreadLibraryCalls(hModule);
            HANDLE hThread = CreateThread(nullptr, 0, Main, hModule, 0, nullptr);
            if (hThread) {
                CloseHandle(hThread);
            }
            break;
        }
    }
    return TRUE;
}