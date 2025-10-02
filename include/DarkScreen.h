#pragma once
#include <windows.h>
#include "Resource.h"

class DarkScreen {
private:
    HWND hwndMain;
    BOOL bScreensaverActive;
    int spriteX;
    int spriteY;

public:
    DarkScreen(HWND hwnd);
    ~DarkScreen();
    
    void StartScreensaver();
    void StopScreensaver();
    void OnKeyDown(WPARAM wParam);
    void DrawSprite(HDC hdc, int x, int y);
    void OnPaint(HDC hdc);
    
    BOOL IsActive() const { return bScreensaverActive; }
};
