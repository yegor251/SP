#include "DllFunctionManager.h"
#include <shlwapi.h>

DllFunctionManager::~DllFunctionManager() {
    for (auto& item : dlls) {
        if (item.hModule) {
            FreeLibrary(item.hModule);
        }
    }
}

bool DllFunctionManager::IsLoaded(const std::wstring& path) const {
    for (const auto& item : dlls) if (item.dllPath == path) return true;
    return false;
}

const DllFunctionItem* DllFunctionManager::GetItem(size_t idx) const {
    if (idx >= dlls.size()) return nullptr;
    return &dlls[idx];
}

bool DllFunctionManager::AddLibrary(const std::wstring& path) {
    if (IsLoaded(path)) return false;
    HMODULE mod = LoadLibraryW(path.c_str());
    if (!mod) return false;
    FARPROC procRaw = GetProcAddress(mod, "dll_entry");
    if (!procRaw) { FreeLibrary(mod); return false; }
    DllProcType fp = reinterpret_cast<DllProcType>(procRaw);
    DllFunctionItem item;
    item.dllPath = path;
    const wchar_t* fname = PathFindFileNameW(path.c_str());
    if (fname) item.dllName = fname; else item.dllName = path;
    item.hModule = mod;
    item.proc = fp;
    dlls.push_back(std::move(item));
    return true;
}
