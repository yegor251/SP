#pragma once
#include <windows.h>
#include <string>

class ClipboardManager {
public:
    static bool CopyToClipboard(const std::wstring& text);
    static std::wstring GetFromClipboard();
    static bool IsClipboardAvailable();
    static void ClearClipboard();
};
