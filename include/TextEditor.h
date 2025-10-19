#pragma once
#include <windows.h>
#include <string>
#include "CellManager.h"
#include "FontManager.h"
#include "ScrollManager.h"
#include "ClipboardHandler.h"

class TextEditor {
private:
    HWND hwndParent;
    HINSTANCE hInstance;
    bool isVisible;
    UINT selectedFontId;
    
    CellManager* cellManager;
    FontManager* fontManager;
    ScrollManager* scrollManager;
    ClipboardHandler* clipboardHandler;
    
public:
    TextEditor(HWND parent, HINSTANCE hInst);
    ~TextEditor();
    
    bool Create();
    void Show();
    void Hide();
    void SetVisible(bool visible);
    bool IsVisible() const { return isVisible; }
    
    void SetText(const std::wstring& text);
    std::wstring GetText() const;
    void Clear();
    void SetEditorFocus();
    void Resize();
    void Cut();
    void Copy();
    void Paste();
    void SelectAll();
    bool IsModified() const;
    void ResetModified();
    
    HWND GetHandle() const;
    
    void OnVScroll(WPARAM wParam, LPARAM lParam);
    void OnMouseWheel(WPARAM wParam, LPARAM lParam);
    void SetFontByMenuId(UINT id);
    void ApplyCurrentFontToFocusedSelection();
    void ApplyFontToAll(const wchar_t* faceName, int heightLogical);
    void SetFontPreset(FontManager::FontPreset preset);
    void ApplySelectedFontToCell(HWND cellHwnd);
    UINT GetSelectedFontId() const { return selectedFontId; }
};
