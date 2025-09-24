#include "TextEditor.h"
#include "ClipboardManager.h"
#include <tchar.h>
#include <string>

TextEditor::TextEditor(HWND parent, HINSTANCE hInst) 
    : hwndEdit(nullptr), hwndParent(parent), hInstance(hInst), isVisible(false), originalEditProc(nullptr) {
}

TextEditor::~TextEditor() {
    if (hwndEdit) {
        DestroyWindow(hwndEdit);
    }
}

bool TextEditor::Create() {
    hwndEdit = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | 
        ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_WANTRETURN,
        0, 0, 0, 0,
        hwndParent,
        (HMENU)1001,
        hInstance,
        nullptr
    );

    if (!hwndEdit) {
        return false;
    }

    originalEditProc = (WNDPROC)SetWindowLongPtr(hwndEdit, GWLP_WNDPROC, (LONG_PTR)EditProc);
    SetWindowLongPtr(hwndEdit, GWLP_USERDATA, (LONG_PTR)this);

    HFONT hFont = CreateFont(
        16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"Consolas"
    );

    if (hFont) {
        SendMessage(hwndEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
    }

    Resize();
    return true;
}

void TextEditor::Show() {
    if (hwndEdit) {
        ShowWindow(hwndEdit, SW_SHOW);
        isVisible = true;
    }
}

void TextEditor::Hide() {
    if (hwndEdit) {
        ShowWindow(hwndEdit, SW_HIDE);
        isVisible = false;
    }
}

void TextEditor::SetVisible(bool visible) {
    if (visible) {
        Show();
    } else {
        Hide();
    }
}

void TextEditor::SetText(const std::wstring& text) {
    if (hwndEdit) {
        SetWindowText(hwndEdit, text.c_str());
    }
}

std::wstring TextEditor::GetText() const {
    if (!hwndEdit) {
        return L"";
    }

    int textLength = GetWindowTextLength(hwndEdit);
    if (textLength == 0) {
        return L"";
    }

    std::wstring text(textLength + 1, L'\0');
    GetWindowText(hwndEdit, &text[0], textLength + 1);
    text.resize(textLength);
    
    return text;
}

void TextEditor::Clear() {
    if (hwndEdit) {
        SetWindowText(hwndEdit, L"");
    }
}

void TextEditor::SetEditorFocus() {
    if (hwndEdit) {
        SetFocus(hwndEdit);
    }
}

void TextEditor::Resize() {
    if (!hwndEdit || !hwndParent) {
        return;
    }

    RECT clientRect;
    if (GetClientRect(hwndParent, &clientRect)) {
        int width = clientRect.right - clientRect.left;
        int height = clientRect.bottom - clientRect.top;
        
        int margin = 10;
        int editorWidth = width - 2 * margin;
        int editorHeight = height - 2 * margin;
        
        if (editorWidth > 0 && editorHeight > 0) {
            SetWindowPos(hwndEdit, nullptr, margin, margin, editorWidth, editorHeight, 
                        SWP_NOZORDER | SWP_NOACTIVATE);
        }
    }
}

LRESULT CALLBACK TextEditor::EditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    TextEditor* pThis = reinterpret_cast<TextEditor*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    
    if (pThis && pThis->originalEditProc) {
        switch (message) {
        case WM_SIZE:
            if (pThis->hwndParent) {
                pThis->Resize();
            }
            break;
            
        case WM_CHAR:
            if (wParam == VK_ESCAPE) {
                pThis->Hide();
                return 0;
            }
            break;
            
        case WM_KEYDOWN:
            {
                bool ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
                
                if (ctrlPressed) {
                    switch (wParam) {
                    case 'A':
                        pThis->SelectAll();
                        return 0;
                    }
                }
            }
            break;
        
        case WM_PASTE:
            // Позволяем стандартной обработке выполнить вставку один раз
            return CallWindowProc(pThis->originalEditProc, hwnd, message, wParam, lParam);
        case WM_COPY:
        case WM_CUT:
            return CallWindowProc(pThis->originalEditProc, hwnd, message, wParam, lParam);
        }
        
        return CallWindowProc(pThis->originalEditProc, hwnd, message, wParam, lParam);
    }
    
    return DefWindowProc(hwnd, message, wParam, lParam);
}

void TextEditor::Cut() {
    if (!hwndEdit) {
        return;
    }

    int start, end;
    SendMessage(hwndEdit, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
    
    if (start != end) {
        int textLength = GetWindowTextLength(hwndEdit);
        if (textLength > 0) {
            std::wstring text(textLength + 1, L'\0');
            GetWindowText(hwndEdit, &text[0], textLength + 1);
            text.resize(textLength);
            
            std::wstring selectedText = text.substr(start, end - start);
            if (ClipboardManager::CopyToClipboard(selectedText)) {
                SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)L"");
            }
        }
    }
}

void TextEditor::Copy() {
    if (!hwndEdit) {
        return;
    }

    int start, end;
    SendMessage(hwndEdit, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
    
    if (start != end) {
        int textLength = GetWindowTextLength(hwndEdit);
        if (textLength > 0) {
            std::wstring text(textLength + 1, L'\0');
            GetWindowText(hwndEdit, &text[0], textLength + 1);
            text.resize(textLength);
            
            std::wstring selectedText = text.substr(start, end - start);
            ClipboardManager::CopyToClipboard(selectedText);
        }
    }
}

void TextEditor::Paste() {
    if (!hwndEdit) {
        return;
    }

    if (ClipboardManager::IsClipboardAvailable()) {
        std::wstring clipboardText = ClipboardManager::GetFromClipboard();
        if (!clipboardText.empty()) {
            SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)clipboardText.c_str());
        }
    }
}

void TextEditor::SelectAll() {
    if (!hwndEdit) {
        return;
    }

    SendMessage(hwndEdit, EM_SETSEL, 0, -1);
}
