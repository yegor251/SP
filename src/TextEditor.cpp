#include "TextEditor.h"
#include "ClipboardManager.h"
#include "Resource.h"
#include <tchar.h>
#include <string>
#include <Richedit.h>
#include <RichEdit.h>

TextEditor::TextEditor(HWND parent, HINSTANCE hInst) 
    : hwndParent(parent), hInstance(hInst), isVisible(false), selectedFontId(IDM_FONT_DEFAULT),
      cellManager(nullptr), fontManager(nullptr), scrollManager(nullptr), clipboardHandler(nullptr) {
}

TextEditor::~TextEditor() {
    if (clipboardHandler) {
        delete clipboardHandler;
        clipboardHandler = nullptr;
    }
    if (scrollManager) {
        delete scrollManager;
        scrollManager = nullptr;
    }
    if (fontManager) {
        delete fontManager;
        fontManager = nullptr;
    }
    if (cellManager) {
        delete cellManager;
        cellManager = nullptr;
    }
}

bool TextEditor::Create() {
    if (!hwndParent || !hInstance) {
        return false;
    }

    cellManager = new CellManager(hwndParent, hInstance);
    if (!cellManager->Create()) {
        return false;
    }
    cellManager->SetTextEditor(this);

    fontManager = new FontManager();
    if (!fontManager->CreateDefaultFont()) {
        return false;
    }
    cellManager->SetFont(fontManager->GetFont());

    scrollManager = new ScrollManager(hwndParent, hInstance, cellManager);
    if (!scrollManager->Create()) {
        return false;
    }

    clipboardHandler = new ClipboardHandler(hwndParent, cellManager);
    
    cellManager->SetContentChangedCallback([](void* userData) {
        TextEditor* editor = static_cast<TextEditor*>(userData);
        if (editor->scrollManager) {
            editor->scrollManager->SetContentHeight(editor->cellManager->GetContentHeight());
            editor->scrollManager->UpdateScrollBarSize();
        }
    }, this);

    Resize();
    return true;
}

void TextEditor::SetFontByMenuId(UINT id) {
    selectedFontId = id;
    if (fontManager) {
        fontManager->SetFontByMenuId(id);
    }
}

void TextEditor::ApplyCurrentFontToFocusedSelection() {
    if (fontManager) {
        fontManager->ApplyCurrentFontToFocusedSelection();
    }
}

void TextEditor::ApplyFontToAll(const wchar_t* faceName, int heightLogical) {
    if (fontManager && cellManager) {
        fontManager->ApplyFontToAll(faceName, heightLogical);
        cellManager->SetFont(fontManager->GetFont());
        cellManager->RelayoutGrid();
        if (scrollManager) {
            scrollManager->SetContentHeight(cellManager->GetContentHeight());
            scrollManager->UpdateScrollBarSize();
        }
    }
}

void TextEditor::SetFontPreset(FontManager::FontPreset preset) {
    if (fontManager && cellManager) {
        fontManager->SetFontPreset(preset);
        cellManager->SetFont(fontManager->GetFont());
        cellManager->RelayoutGrid();
        if (scrollManager) {
            scrollManager->SetContentHeight(cellManager->GetContentHeight());
            scrollManager->UpdateScrollBarSize();
        }
    }
}

void TextEditor::ApplySelectedFontToCell(HWND cellHwnd) {
    if (!cellHwnd || !fontManager) {
        return;
    }
    
    const wchar_t* faceName = L"Consolas";
    if (selectedFontId == IDM_FONT_RASTER) {
        faceName = L"Courier New";
    } else if (selectedFontId == IDM_FONT_VECTOR) {
        faceName = L"Arial";
    }
    
    CHARFORMAT2 cf = {};
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_FACE | CFM_SIZE | CFM_WEIGHT;
    cf.yHeight = 16 * 20;
    cf.wWeight = FW_NORMAL;
    wcscpy_s(cf.szFaceName, LF_FACESIZE, faceName);

    int textLength = GetWindowTextLength(cellHwnd);
    SendMessage(cellHwnd, EM_SETSEL, textLength, textLength);
    
    SendMessage(cellHwnd, EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cf);
}

void TextEditor::Show() {
    if (cellManager) {
        cellManager->Show();
        isVisible = true;
    }
}

void TextEditor::Hide() {
    if (cellManager) {
        cellManager->Hide();
        isVisible = false;
    }
}

void TextEditor::SetVisible(bool visible) {
    if (cellManager) {
        cellManager->SetVisible(visible);
        isVisible = visible;
    }
}

void TextEditor::SetText(const std::wstring& text) {
    if (cellManager) {
        cellManager->SetText(text);
        if (scrollManager) {
            scrollManager->SetContentHeight(cellManager->GetContentHeight());
            scrollManager->UpdateScrollBarSize();
        }
    }
}

std::wstring TextEditor::GetText() const {
    if (cellManager) {
        return cellManager->GetText();
    }
    return L"";
}

void TextEditor::Clear() {
    if (cellManager) {
        cellManager->Clear();
    }
}

void TextEditor::SetEditorFocus() {
    if (cellManager) {
        cellManager->SetEditorFocus();
    }
}

void TextEditor::Resize() {
    if (cellManager) {
        cellManager->Resize();
        if (scrollManager) {
            RECT rc;
            if (GetClientRect(hwndParent, &rc)) {
                scrollManager->SetContentHeight(cellManager->GetContentHeight());
                scrollManager->UpdateScrollBar(rc);
            }
        }
    }
}

void TextEditor::OnVScroll(WPARAM wParam, LPARAM lParam) {
    if (scrollManager) {
        scrollManager->OnVScroll(wParam, lParam);
    }
}

void TextEditor::OnMouseWheel(WPARAM wParam, LPARAM lParam) {
    if (scrollManager) {
        scrollManager->OnMouseWheel(wParam, lParam);
    }
}

void TextEditor::Cut() {
    if (clipboardHandler) {
        clipboardHandler->Cut();
    }
}

void TextEditor::Copy() {
    if (clipboardHandler) {
        clipboardHandler->Copy();
    }
}

void TextEditor::Paste() {
    if (clipboardHandler) {
        clipboardHandler->Paste();
    }
}

void TextEditor::SelectAll() {
    if (clipboardHandler) {
        clipboardHandler->SelectAll();
    }
}

bool TextEditor::IsModified() const {
    if (cellManager) {
        return cellManager->IsModified();
    }
    return false;
}

void TextEditor::ResetModified() {
    if (cellManager) {
        cellManager->ResetModified();
    }
}

HWND TextEditor::GetHandle() const {
    if (cellManager) {
        return cellManager->GetHandle();
    }
    return nullptr;
}
