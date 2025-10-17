#pragma once
#include <windows.h>
#include <string>

class CellManager;

class ClipboardHandler {
private:
    HWND hwndParent;
    CellManager* cellManager;

public:
    ClipboardHandler(HWND parent, CellManager* cellMgr);
    
    void Cut();
    void Copy();
    void Paste();
    void SelectAll();
    
    bool IsClipboardAvailable() const;
    std::wstring GetFromClipboard() const;
    bool CopyToClipboard(const std::wstring& text) const;
    
private:
    HWND GetTargetWindow() const;
    std::wstring GetSelectedText(HWND hwnd) const;
};
