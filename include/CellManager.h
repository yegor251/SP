#pragma once
#include <windows.h>
#include <string>

class TextEditor;

class CellManager {
private:
    static const int kRows = 10;
    static const int kCols = 4;
    static const int kDefaultRowHeight = 27;

    HWND cellEdits[kRows][kCols];
    int rowHeights[kRows];
    HWND hwndParent;
    HINSTANCE hInstance;
    HMODULE hMsftedit;
    int scrollOffset;
    
    typedef void (*ContentChangedCallback)(void* userData);
    ContentChangedCallback contentChangedCallback;
    void* contentChangedUserData;
    
    TextEditor* textEditor;

public:
    CellManager(HWND parent, HINSTANCE hInst);
    ~CellManager();
    
    bool Create();
    void Destroy();
    void Show();
    void Hide();
    void SetVisible(bool visible);
    bool IsVisible() const;
    
    void SetText(const std::wstring& text);
    std::wstring GetText() const;
    void Clear();
    void SetEditorFocus();
    void Resize();
    
    bool IsModified() const;
    void ResetModified();
    bool IsOurCell(HWND hwnd) const;
    HWND GetHandle() const;
    
    void SetFont(HFONT font);
    void SetCharFormat(const wchar_t* faceName, int height);
    void SetCharFormatForCell(int row, int col, const wchar_t* faceName, int height);
    void SetCharFormatForCellByHandle(HWND hwnd, const wchar_t* faceName, int height);
    void RelayoutGrid();
    void RelayoutGridWithScrollOffset(int scrollOffset);
    int GetContentHeight() const;
    void SetContentChangedCallback(ContentChangedCallback callback, void* userData);
    void SetTextEditor(TextEditor* editor);

    void SetRowHeight(int row, int height);
    int GetRowHeight(int row) const;
    void IncreaseRowHeight(int row, int increment = kDefaultRowHeight);
    void CheckAndAdjustRowHeight(int row);
    
    static LRESULT CALLBACK EditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    
private:
    WNDPROC GetOriginalProc(HWND hwnd) const;
    bool SetOriginalProc(HWND hwnd, WNDPROC proc);

public:
    bool ReplaceWithDashesForAllCells(const std::wstring& targetW);
    void ApplySelectedFontToAllCells();
};
