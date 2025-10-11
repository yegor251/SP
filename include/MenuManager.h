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

public:
    MenuManager();
    ~MenuManager();
    
    HMENU CreateMainMenu();
    void CleanupMenu();
};
