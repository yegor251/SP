#pragma once
#include <windows.h>
#include "MenuManager.h"
#include "AboutDialog.h"
#include "Resource.h"
#include "DarkScreen.h"
#include "TextEditor.h"

class MainWindow {
private:
    HWND hwndMain;
    HINSTANCE hInstance;
    MenuManager menuManager;
    DarkScreen* darkScreen;
    TextEditor* textEditor;
    POINT lastMousePos;
    std::wstring currentFilePath;
    class DllFunctionManager* dllManager;
    
public:
    MainWindow(HINSTANCE hInst);
    ~MainWindow();
    
    bool Create();
    void Show(int nCmdShow);
    HWND GetHandle() const { return hwndMain; }
    
private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void OnCreate();
    void OnPaint();
    void OnCommand(WPARAM wParam);
    void OnMouseMove(int x, int y);
    void OnKeyDown(WPARAM wParam);
    void OnLeftMouseClick(int x, int y);
    void OnMouseClick();
    void OnTimer(WPARAM wParam);
    void OnSize();
};
