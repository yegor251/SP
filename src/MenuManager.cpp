#include "MenuManager.h"

MenuManager::MenuManager() : hMainMenu(nullptr), hFileMenu(nullptr), hEditMenu(nullptr), hHelpMenu(nullptr) {
}

MenuManager::~MenuManager() {
    CleanupMenu();
}

HMENU MenuManager::CreateMainMenu() {
    hMainMenu = CreateMenu();
    
    hFileMenu = CreatePopupMenu();
    AppendMenu(hFileMenu, MF_STRING, IDM_FILE_EXIT, L"Exit");
    AppendMenu(hMainMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"File");
    
    hEditMenu = CreatePopupMenu();
    AppendMenu(hEditMenu, MF_STRING, IDM_EDIT_CUT, L"Cut");
    AppendMenu(hEditMenu, MF_STRING, IDM_EDIT_COPY, L"Copy");
    AppendMenu(hEditMenu, MF_STRING, IDM_EDIT_PASTE, L"Paste");
    AppendMenu(hEditMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(hEditMenu, MF_STRING, IDM_EDIT_SELECT_ALL, L"Select All");
    AppendMenu(hMainMenu, MF_POPUP, (UINT_PTR)hEditMenu, L"Edit");
    
    hHelpMenu = CreatePopupMenu();
    AppendMenu(hHelpMenu, MF_STRING, IDM_HELP_ABOUT, L"About");
    AppendMenu(hMainMenu, MF_POPUP, (UINT_PTR)hHelpMenu, L"Help");
    
    return hMainMenu;
}

void MenuManager::CleanupMenu() {
    if (hMainMenu) {
        ::DestroyMenu(hMainMenu);
        hMainMenu = nullptr;
    }
    hFileMenu = nullptr;
    hEditMenu = nullptr;
    hHelpMenu = nullptr;
}
