#include "DarkScreen.h"
#include "TextEditor.h"

DarkScreen::DarkScreen(HWND hwnd, TextEditor* editor) 
    : hwndMain(hwnd), bScreensaverActive(FALSE), textEditor(editor) {
    RECT rect;
    if (GetClientRect(hwndMain, &rect)) {
        spriteX = rect.right / 2;
        spriteY = rect.bottom / 2;
    } else {
        spriteX = 100;
        spriteY = 100;
    }
}

DarkScreen::~DarkScreen() {
    StopScreensaver();
}

void DarkScreen::StartScreensaver() {
    if (!bScreensaverActive) {
        bScreensaverActive = TRUE;

        if (!ShowCursor(FALSE)) {
        }

        RECT rect;
        if (GetClientRect(hwndMain, &rect)) {
            spriteX = rect.right / 2;
            spriteY = rect.bottom / 2;
        }

        HWND hwndEdit = FindWindowEx(hwndMain, nullptr, L"EDIT", nullptr);
        if (hwndEdit) {
            if (!ShowWindow(hwndEdit, SW_HIDE)) {
            }
        }

        if (textEditor && textEditor->GetCellManager()) {
            textEditor->GetCellManager()->Hide();
        }

        if (!InvalidateRect(hwndMain, nullptr, TRUE)) {
        }
    }
}

void DarkScreen::StopScreensaver() {
    if (bScreensaverActive) {
        bScreensaverActive = FALSE;
        
        if (!ShowCursor(TRUE)) {
        }
        
        HWND hwndEdit = FindWindowEx(hwndMain, nullptr, L"EDIT", nullptr);
        if (hwndEdit) {
            if (!ShowWindow(hwndEdit, SW_SHOW)) {
            }
        }
        
        if (textEditor && textEditor->GetCellManager()) {
            textEditor->GetCellManager()->Show();
        }
        
        if (!InvalidateRect(hwndMain, nullptr, TRUE)) {
        }
    }
}

void DarkScreen::OnKeyDown(WPARAM wParam) {
    if (!bScreensaverActive) {
        return;
    }

    RECT rect;
    if (!GetClientRect(hwndMain, &rect)) {
        return;
    }

    int newX = spriteX;
    int newY = spriteY;

    switch (wParam) {
        case 'W':
            newY -= SPRITE_SPEED;
            break;
        case 'S':
            newY += SPRITE_SPEED;
            break;
        case 'A':
            newX -= SPRITE_SPEED;
            break;
        case 'D':
            newX += SPRITE_SPEED;
            break;
        default:
            return;
    }

    if (newX >= 0 && newX + SPRITE_SIZE <= rect.right) {
        spriteX = newX;
    }
    if (newY >= 0 && newY + SPRITE_SIZE <= rect.bottom) {
        spriteY = newY;
    }

    if (!InvalidateRect(hwndMain, nullptr, TRUE)) {
    }
}

void DarkScreen::DrawSprite(HDC hdc, int x, int y) {
    HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 255));
    if (!hBrush) {
        return;
    }
    
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
    if (!hOldBrush) {
        DeleteObject(hBrush);
        return;
    }

    if (!Ellipse(hdc, x, y, x + SPRITE_SIZE, y + SPRITE_SIZE)) {
    }

    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrush);
}

void DarkScreen::OnPaint(HDC hdc) {
    RECT rect;
    if (!GetClientRect(hwndMain, &rect)) {
        return;
    }
    
    if (bScreensaverActive) {
        if (!FillRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH))) {
        }
        DrawSprite(hdc, spriteX, spriteY);
    }
}
