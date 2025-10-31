#pragma once
#include <windows.h>
#include "DllFunctionManager.h"
#include <functional>
#include <string>

class DllDialog {
public:
    using ExecuteCallback = std::function<void(int /*index*/)>;
    static void Show(HWND hwndParent, DllFunctionManager& manager, ExecuteCallback onExecute, std::function<void(const std::wstring&)> onAddDll);
};
