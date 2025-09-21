#pragma once
#include <windows.h>
#include <string>

class TextEditor {
private:
    HWND hwndEdit;
    HWND hwndParent;
    HINSTANCE hInstance;
    bool isVisible;
    
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
    
    HWND GetHandle() const { return hwndEdit; }
    
private:
    static LRESULT CALLBACK EditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    WNDPROC originalEditProc;
};
