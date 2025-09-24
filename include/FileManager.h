#pragma once
#include <windows.h>
#include <string>

namespace FileManager {
    bool OpenTextFile(HWND owner, std::wstring& outText, std::wstring& outPath);
    bool SaveTextFile(HWND owner, const std::wstring& text, std::wstring& outPath);
    bool SaveTextFileToPath(const std::wstring& path, const std::wstring& text);
    int ConfirmSaveChanges(HWND owner);
}


