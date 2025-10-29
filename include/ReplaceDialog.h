#pragma once
#include <windows.h>
#include <string>
#include <functional>

class ReplaceDialog {
public:
    using ExecuteCallback = std::function<void(const std::wstring&)>;
    static void Show(HWND hwndParent, ExecuteCallback onExecute);
};


