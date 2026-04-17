/*
Copyright (c) Anthony Beaumont
This source code is licensed under the MIT License
found in the LICENSE file in the root directory of this source tree.
*/

#pragma once
#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include <string>
#include <iostream>

std::wstring Getenv(LPCWSTR name);
std::wstring GetLastErrorMessage();
void enableConsole();