/*
Copyright (c) Anthony Beaumont
This source code is licensed under the MIT License
found in the LICENSE file in the root directory of this source tree.

This file contains code derived from: https://github.com/xan105/AnyLua
Copyright (c) Anthony Beaumont. Licensed under the MIT License.

cf: https://github.com/xan105/AnyLua/blob/main/src/lua/module/memory/memory.cpp
*/

#pragma once
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>
#include <psapi.h>

namespace memory {
    std::vector<uintptr_t> find(const std::string& moduleName, const std::string& patternHexStr, const std::string& mode);
    bool write(uintptr_t baseAddress, int offset, const std::string& valueHexStr);
}