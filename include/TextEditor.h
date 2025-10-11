#pragma once
#include <windows.h>
#include <string>

class TextEditor {
private:
    static const int kRows = 10;
    static const int kCols = 4;
    HWND cellEdits[kRows][kCols];
    HWND hwndVScroll;
    HWND hwndParent;
    HINSTANCE hInstance;
    bool isVisible;
    HFONT hFont;
    int scrollOffsetY;
    int contentHeight;
    HMODULE hMsftedit;
    enum class FontChoice { DefaultConsolas, RasterTerminal, VectorArial } currentFont;
    
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
    
    HWND GetHandle() const { return cellEdits[0][0]; }
    
private:
    static LRESULT CALLBACK EditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    WNDPROC GetOriginalProc(HWND hwnd) const;
    bool SetOriginalProc(HWND hwnd, WNDPROC proc);
    bool IsOurCell(HWND hwnd) const;
    void RelayoutGrid();
    int CalculateCellTextHeight(HWND hwndCell, int availableWidth) const;
    void UpdateScrollBar(const RECT& clientRect);
    void ScrollBy(int deltaY);
public:
    void OnVScroll(WPARAM wParam, LPARAM lParam);
    void SetFontByMenuId(UINT id);
    void ApplyCurrentFontToFocusedSelection();
    enum class FontPreset { DefaultConsolas, RasterTerminal, VectorArial };
    void ApplyFontToAll(const wchar_t* faceName, int heightLogical);
    void SetFontPreset(FontPreset preset);
};
