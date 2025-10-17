#include "ScrollManager.h"
#include "CellManager.h"

ScrollManager::ScrollManager(HWND parent, HINSTANCE hInst, CellManager* cellMgr) 
    : hwndParent(parent), hInstance(hInst), hwndVScroll(nullptr), scrollOffsetY(0), contentHeight(0), cellManager(cellMgr) {
}

ScrollManager::~ScrollManager() {
    Destroy();
}

bool ScrollManager::Create() {
    if (!hwndParent || !hInstance) {
        return false;
    }
    
    hwndVScroll = CreateWindowEx(0, L"SCROLLBAR", nullptr,
                                 WS_CHILD | WS_VISIBLE | SBS_VERT,
                                 0, 0, 0, 0, hwndParent, nullptr, hInstance, nullptr);
    return hwndVScroll != nullptr;
}

void ScrollManager::Destroy() {
    if (hwndVScroll) {
        DestroyWindow(hwndVScroll);
        hwndVScroll = nullptr;
    }
}

void ScrollManager::UpdateScrollBar(const RECT& clientRect) {
    if (!hwndVScroll) {
        return;
    }
    
    int scrollBarWidth = GetSystemMetrics(SM_CXVSCROLL);
    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;
    
    SetWindowPos(hwndVScroll, nullptr, 
                 width - scrollBarWidth, 0, 
                 scrollBarWidth, height,
                 SWP_NOZORDER | SWP_NOACTIVATE);
    
    int viewHeight = height - 2 * 10;
    if (viewHeight < 0) {
        viewHeight = 0;
    }
    
    SCROLLINFO si = {};
    si.cbSize = sizeof(si);
    si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
    si.nMin = 0;
    si.nMax = contentHeight;
    si.nPage = viewHeight;
    
    if (contentHeight <= viewHeight) {
        ShowWindow(hwndVScroll, SW_HIDE);
        scrollOffsetY = 0;
        if (cellManager) {
            cellManager->RelayoutGridWithScrollOffset(0);
        }
        return;
    } else {
        ShowWindow(hwndVScroll, SW_SHOW);
    }
    
    if (scrollOffsetY < 0) {
        scrollOffsetY = 0;
    }
    int maxOffset = contentHeight - viewHeight;
    if (maxOffset < 0) {
        maxOffset = 0;
    }
    if (scrollOffsetY > maxOffset) {
        scrollOffsetY = maxOffset;
    }
    si.nPos = scrollOffsetY;
    SetScrollInfo(hwndVScroll, SB_CTL, &si, TRUE);
}

void ScrollManager::ScrollBy(int deltaY) {
    if (!hwndVScroll || !IsWindowVisible(hwndVScroll)) {
        return;
    }
    
    int newPos = scrollOffsetY + deltaY;
    RECT rc;
    if (!GetClientRect(hwndParent, &rc)) {
        return;
    }
    int viewHeight = rc.bottom - rc.top - 2 * 10;
    int maxOffset = contentHeight - viewHeight;
    if (maxOffset < 0) {
        maxOffset = 0;
    }
    if (newPos < 0) newPos = 0;
    if (newPos > maxOffset) newPos = maxOffset;
    if (newPos != scrollOffsetY) {
        scrollOffsetY = newPos;
        SCROLLINFO si = {};
        si.cbSize = sizeof(si);
        si.fMask = SIF_POS;
        si.nPos = scrollOffsetY;
        SetScrollInfo(hwndVScroll, SB_CTL, &si, TRUE);
        if (cellManager) {
            cellManager->RelayoutGridWithScrollOffset(scrollOffsetY);
        }
    }
}

void ScrollManager::OnVScroll(WPARAM wParam, LPARAM lParam) {
    if (!hwndVScroll || (HWND)lParam != hwndVScroll) {
        return;
    }
    
    if (!IsWindowVisible(hwndVScroll)) {
        return;
    }
    
    SCROLLINFO si = {};
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    GetScrollInfo(hwndVScroll, SB_CTL, &si);
    int pos = si.nPos;
    
    switch (LOWORD(wParam)) {
    case SB_LINEUP: pos -= 20; break;
    case SB_LINEDOWN: pos += 20; break;
    case SB_PAGEUP: pos -= (int)si.nPage; break;
    case SB_PAGEDOWN: pos += (int)si.nPage; break;
    case SB_THUMBTRACK: pos = si.nTrackPos; break;
    case SB_THUMBPOSITION: pos = si.nTrackPos; break;
    default: break;
    }
    
    RECT rc;
    GetClientRect(hwndParent, &rc);
    int viewH = rc.bottom - rc.top - 2 * 10;
    int maxOffset = contentHeight - viewH;
    if (maxOffset < 0) maxOffset = 0;
    if (pos < 0) pos = 0;
    if (pos > maxOffset) pos = maxOffset;
    
    if (pos != si.nPos) {
        si.fMask = SIF_POS;
        si.nPos = pos;
        SetScrollInfo(hwndVScroll, SB_CTL, &si, TRUE);
        scrollOffsetY = pos;
        if (cellManager) {
            cellManager->RelayoutGridWithScrollOffset(scrollOffsetY);
        }
    }
}

void ScrollManager::OnMouseWheel(WPARAM wParam, LPARAM lParam) {
    if (!hwndVScroll || !IsWindowVisible(hwndVScroll)) {
        return;
    }
    
    int delta = GET_WHEEL_DELTA_WPARAM(wParam);
    int scrollLines = 3;
    int deltaY = -(delta / WHEEL_DELTA) * scrollLines * 20;
    ScrollBy(deltaY);
}

void ScrollManager::SetContentHeight(int height) {
    contentHeight = height;
}

void ScrollManager::SetScrollOffset(int offset) {
    scrollOffsetY = offset;
}

void ScrollManager::UpdateScrollBarSize() {
    if (!hwndVScroll || !hwndParent) {
        return;
    }
    
    RECT rc;
    if (!GetClientRect(hwndParent, &rc)) {
        return;
    }
    
    UpdateScrollBar(rc);
}
