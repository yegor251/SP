#pragma once
#include <windows.h>
#include "Resource.h"

class DarkScreen {
private:
    HWND hwndMain;
    BOOL bScreensaverActive;
    int spriteX;
    int spriteY;
    int spriteDirectionX;
    int spriteDirectionY;

public:
    DarkScreen(HWND hwnd);
    ~DarkScreen();
    
    void StartInactivityTimer();
    void StopInactivityTimer();
    void StartScreensaver();
    void StopScreensaver();
    void OnTimer(WPARAM wParam);
    void OnUserActivity();
    void DrawSprite(HDC hdc, int x, int y);
    void OnPaint(HDC hdc);
    
    BOOL IsActive() const { return bScreensaverActive; }
};
