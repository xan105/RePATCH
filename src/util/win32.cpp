/*
Copyright (c) Anthony Beaumont
This source code is licensed under the MIT License
found in the LICENSE file in the root directory of this source tree.
*/

#include "win32.h"

std::wstring Getenv(LPCWSTR name) {
    std::wstring buffer(65535, L'\0');
    DWORD size = GetEnvironmentVariableW(name, &buffer[0], static_cast<DWORD>(buffer.size()));

    if (size) {
        buffer.resize(size);
        return buffer;
    }
    else {
        return L"";
    }
}

std::wstring GetLastErrorMessage() {
    DWORD code = ::GetLastError();
    LPWSTR buffer = nullptr;
    size_t size = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&buffer,
        0,
        NULL
    );
    std::wstring message(buffer, size);
    LocalFree(buffer);
    return L"Error " + std::to_wstring(code) + L": " + message;
}

void enableConsole() {
    if (AllocConsole()) {
        HWND consoleWindow = GetConsoleWindow();

        if (consoleWindow) {
            // Set the console window to be layered
            LONG style = GetWindowLong(consoleWindow, GWL_EXSTYLE);
            SetWindowLong(consoleWindow, GWL_EXSTYLE, style | WS_EX_LAYERED);
            SetLayeredWindowAttributes(consoleWindow, 0, 225, LWA_COLORKEY);

            // Show the console window
            ShowWindow(consoleWindow, SW_SHOW);
        }

        // Redirect stdio to the console
        FILE* dummy;
        freopen_s(&dummy, "CONOUT$", "w", stdout);
        freopen_s(&dummy, "CONIN$", "r", stdin);
        freopen_s(&dummy, "CONOUT$", "w", stderr);
    }
}