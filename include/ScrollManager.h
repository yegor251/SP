#pragma once
#include <windows.h>

class CellManager;

class ScrollManager {
private:
    HWND hwndVScroll;
    HWND hwndParent;
    HINSTANCE hInstance;
    int scrollOffsetY;
    int contentHeight;
    CellManager* cellManager;

public:
    ScrollManager(HWND parent, HINSTANCE hInst, CellManager* cellMgr);
    ~ScrollManager();
    
    bool Create();
    void Destroy();
    
    void UpdateScrollBar(const RECT& clientRect);
    void ScrollBy(int deltaY);
    void OnVScroll(WPARAM wParam, LPARAM lParam);
    void OnMouseWheel(WPARAM wParam, LPARAM lParam);
    void SetContentHeight(int height);
    int GetScrollOffset() const { return scrollOffsetY; }
    void SetScrollOffset(int offset);
    void UpdateScrollBarSize();
    
    HWND GetScrollHandle() const { return hwndVScroll; }
};
