#include "DarkScreen.h"
#include <tchar.h>

DarkScreen::DarkScreen(HWND hwnd) 
    : hwndMain(hwnd), bScreensaverActive(FALSE) {
    spriteX = 0;
    spriteY = 0;
    spriteDirectionX = 1;
    spriteDirectionY = 1;
}

DarkScreen::~DarkScreen() {
    StopInactivityTimer();
    StopScreensaver();
}

void DarkScreen::StartInactivityTimer() {
    SetTimer(hwndMain, IDT_INACTIVITY_TIMER, 30000, nullptr);
}

void DarkScreen::StopInactivityTimer() {
    KillTimer(hwndMain, IDT_INACTIVITY_TIMER);
}

void DarkScreen::StartScreensaver() {
    if (!bScreensaverActive) {
        bScreensaverActive = TRUE;
        StopInactivityTimer();

        ShowCursor(FALSE);
        SetTimer(hwndMain, IDT_SPRITE_TIMER, 50, nullptr);

        RECT rect;
        GetClientRect(hwndMain, &rect);
        spriteX = rect.right / 2;
        spriteY = rect.bottom / 2;

        InvalidateRect(hwndMain, nullptr, TRUE);
    }
}

void DarkScreen::StopScreensaver() {
    if (bScreensaverActive) {
        bScreensaverActive = FALSE;
        KillTimer(hwndMain, IDT_SPRITE_TIMER);
        ShowCursor(TRUE);
        StartInactivityTimer();
        InvalidateRect(hwndMain, nullptr, TRUE);
    }
}

void DarkScreen::OnTimer(WPARAM wParam) {
    if (wParam == IDT_INACTIVITY_TIMER) {
        StartScreensaver();
    } else if (wParam == IDT_SPRITE_TIMER && bScreensaverActive) {
        RECT rect;
        GetClientRect(hwndMain, &rect);

        spriteX += SPRITE_SPEED * spriteDirectionX;
        spriteY += SPRITE_SPEED * spriteDirectionY;

        if (spriteX <= 0 || spriteX + SPRITE_SIZE >= rect.right) {
            spriteDirectionX *= -1;
        }
        if (spriteY <= 0 || spriteY + SPRITE_SIZE >= rect.bottom) {
            spriteDirectionY *= -1;
        }

        InvalidateRect(hwndMain, nullptr, TRUE);
    }
}

void DarkScreen::OnUserActivity() {
    if (bScreensaverActive) {
        StopScreensaver();
    } else {
        StopInactivityTimer();
        StartInactivityTimer();
    }
}

void DarkScreen::DrawSprite(HDC hdc, int x, int y) {
    HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 255));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

    Ellipse(hdc, x, y, x + SPRITE_SIZE, y + SPRITE_SIZE);

    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrush);
}

void DarkScreen::OnPaint(HDC hdc) {
    RECT rect;
    GetClientRect(hwndMain, &rect);
    
    if (bScreensaverActive) {
        FillRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
        DrawSprite(hdc, spriteX, spriteY);
    }
}
