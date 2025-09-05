#pragma once
#include <windows.h>
#include "Resource.h"

class MenuManager {
private:
    HMENU hMainMenu;
    HMENU hFileMenu;
    HMENU hEditMenu;
    HMENU hHelpMenu;

public:
    MenuManager();
    ~MenuManager();
    
    HMENU CreateMainMenu();
    void CleanupMenu();
};
