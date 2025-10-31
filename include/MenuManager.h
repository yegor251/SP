#pragma once
#include <windows.h>
#include "Resource.h"

class MenuManager {
private:
    HMENU hMainMenu;
    HMENU hFileMenu;
    HMENU hEditMenu;
    HMENU hHelpMenu;
    HMENU hFontMenu;
    HMENU hDllMenu;

public:
    MenuManager();
    ~MenuManager();
    
    HMENU CreateMainMenu();
    void CleanupMenu();
    HMENU GetDllMenu() const { return hDllMenu; }
};
