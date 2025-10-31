#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>

// C-style экспорт: void dll_entry(const wchar_t* input, wchar_t* outbuf, size_t outsize)
typedef void(__cdecl* DllProcType)(const wchar_t*, wchar_t*, size_t);

struct DllFunctionItem {
    std::wstring dllPath;
    std::wstring dllName;
    HMODULE hModule = nullptr;
    DllProcType proc = nullptr;
};

class DllFunctionManager {
public:
    DllFunctionManager() = default;
    ~DllFunctionManager();

    bool AddLibrary(const std::wstring& path);
    size_t Count() const { return dlls.size(); }
    const std::vector<DllFunctionItem>& GetItems() const { return dlls; }
    bool IsLoaded(const std::wstring& path) const;
    const DllFunctionItem* GetItem(size_t idx) const;
private:
    std::vector<DllFunctionItem> dlls;
};
