/*
Copyright (c) Anthony Beaumont
This source code is licensed under the MIT License
found in the LICENSE file in the root directory of this source tree.

This file contains code derived from: https://github.com/xan105/AnyLua
Copyright (c) Anthony Beaumont. Licensed under the MIT License.

cf: https://github.com/xan105/AnyLua/blob/main/src/lua/module/memory/memory.cpp
*/

#include "memory.h"
#include "./util/win32.h"
#include "./util/string.h"

namespace memory {
    namespace {
        bool Patch(uintptr_t address, const std::vector<BYTE>& patch, HANDLE hProcess = nullptr) 
        {
            if (patch.empty()) return false;
            if (hProcess == nullptr) hProcess = GetCurrentProcess();

            DWORD oldProtect = 0;
            if (!VirtualProtectEx(
                hProcess, 
                reinterpret_cast<LPVOID>(address), 
                static_cast<SIZE_T>(patch.size()), 
                PAGE_EXECUTE_READWRITE, 
                &oldProtect)
            ){
                return false;
            }

            bool success = WriteProcessMemory(
                hProcess, 
                reinterpret_cast<LPVOID>(address), 
                patch.data(), 
                static_cast<SIZE_T>(patch.size()), 
                NULL);

            if (!VirtualProtectEx(
                hProcess, 
                reinterpret_cast<LPVOID>(address), 
                static_cast<SIZE_T>(patch.size()), 
                oldProtect,
                &oldProtect)
            ){         
                return false;
            }

            return success;
          }
    
        std::vector<uintptr_t> FindPattern(uintptr_t baseAddress, size_t sizeOfImage, const std::vector<int>& pattern) {
            std::vector<uintptr_t> result{};
            if (pattern.empty() || sizeOfImage < pattern.size()) return result;
            
            uintptr_t current    = baseAddress;
            uintptr_t maxAddress = baseAddress + sizeOfImage;

            MEMORY_BASIC_INFORMATION mbi;
            while (current < maxAddress) {
                if (VirtualQuery(reinterpret_cast<LPCVOID>(current), &mbi, sizeof(mbi)) == 0) {
                    std::wstring error = GetLastErrorMessage();
                    throw std::runtime_error(toString(error));
                }
                uintptr_t nextPage = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
                if (mbi.State == MEM_COMMIT && !(mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD))) { // Skip irrelevant code regions
                    uintptr_t start = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
                    uintptr_t end   = nextPage - pattern.size();
                
                    for (uintptr_t i = start; i <= end; ++i) {
                        bool match = true;
                        auto* ptr = reinterpret_cast<const BYTE*>(i);
                        for (size_t j = 0; j < pattern.size(); ++j) {
                            int byte = pattern[j];
                            if (byte != -1 && ptr[j] != byte) {
                                match = false;
                                break;
                            }
                        }
                        if (match) {
                            result.push_back(i);
                        }   
                    }
                }
                current = nextPage;
            }
            return result;
        }
          
        template <typename T> std::vector<T> ParseHexStringTo(const std::string& input) {
            static_assert(
                std::is_same_v<T, int> || std::is_same_v<T, BYTE>,
                "ParseHexStringTo<T>: T must be either std::vector<int> or std::vector<BYTE>"
              );
              
              std::string hex;
              hex.reserve(input.size());
              // Remove whitespace
              std::remove_copy_if(input.begin(), input.end(), std::back_inserter(hex), [](unsigned char c) { return std::isspace(c); });

              if (hex.size() % 2 != 0) throw std::invalid_argument("Input must have an even number of characters");

              std::vector<T> result;
              result.reserve(hex.size() / 2);

              for (size_t i = 0; i < hex.size(); i += 2) {
                  char high = hex[i];
                  char low = hex[i + 1];

                  if (high == '?' && low == '?') {
                      if constexpr (std::is_same_v<T, int>) {
                          result.push_back(-1); // wildcard marker
                      }
                      else {
                          throw std::invalid_argument("Wildcard not supported! Wrong type?");
                      }
                  }
                  else if (std::isxdigit(high) && std::isxdigit(low)) {
                      int byte = std::stoi(hex.substr(i, 2), nullptr, 16);
                      result.push_back(static_cast<T>(byte));
                  }
                  else {
                      throw std::invalid_argument("Input contains invalid characters");
                  }
              }
              return result;
        }
    
        MODULEINFO GetModuleInfo(const std::wstring& moduleName = L"") {
            HMODULE hModule = nullptr;
            
            if (moduleName.empty()) {
                hModule = GetModuleHandleW(nullptr);
            } else {
                hModule = GetModuleHandleW(moduleName.c_str());
            }
            if (!hModule) {
                std::wstring error = GetLastErrorMessage();
                throw std::invalid_argument(toString(error));
            }

            MODULEINFO moduleInfo = {};
            HANDLE hProcess = GetCurrentProcess();
            if (!GetModuleInformation(hProcess, hModule, &moduleInfo, sizeof(moduleInfo))) {
                std::wstring error = GetLastErrorMessage();
                throw std::runtime_error(toString(error));
            }
            
            return moduleInfo;
        }   
    }

    std::vector<uintptr_t> find(const std::string& moduleName, const std::string& patternHexStr, const std::string& mode) {
        MODULEINFO moduleInfo = GetModuleInfo(toWString(moduleName));
        uintptr_t baseAddress = reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll);
        std::vector<int> pattern = ParseHexStringTo<int>(patternHexStr);

        std::vector<uintptr_t> matches = FindPattern(baseAddress, moduleInfo.SizeOfImage, pattern);
        if (matches.empty()) return {};
        
        if (mode == "first") { 
            return { matches.front() };
        }
        else if (mode == "last") {
            return { matches.back() };
        }

        return matches;
   }
   
    bool write(uintptr_t baseAddress, int offset, const std::string& valueHexStr) {
        std::vector<BYTE> value = ParseHexStringTo<BYTE>(valueHexStr);
        auto address = reinterpret_cast<uintptr_t>(
            reinterpret_cast<char*>(baseAddress) + offset
        );
        return Patch(address, value);
    }
}