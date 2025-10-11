#include "TextEditor.h"
#include "ClipboardManager.h"
#include "Resource.h"
#include <tchar.h>
#include <string>
#include <Richedit.h>
#include <RichEdit.h>

TextEditor::TextEditor(HWND parent, HINSTANCE hInst) 
    : hwndParent(parent), hInstance(hInst), isVisible(false), hFont(nullptr), hwndVScroll(nullptr), scrollOffsetY(0), contentHeight(0), hMsftedit(nullptr), currentFont(FontChoice::DefaultConsolas) {
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            cellEdits[r][c] = nullptr;
        }
    }
}

TextEditor::~TextEditor() {
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (cellEdits[r][c]) {
                DestroyWindow(cellEdits[r][c]);
                cellEdits[r][c] = nullptr;
            }
        }
    }
    if (hFont) {
        DeleteObject(hFont);
        hFont = nullptr;
    }
    if (hMsftedit) {
        FreeLibrary(hMsftedit);
        hMsftedit = nullptr;
    }
}

bool TextEditor::Create() {
    if (!hwndParent || !hInstance) {
        return false;
    }

    hMsftedit = LoadLibrary(L"Msftedit.dll");
    if (!hMsftedit) {
        return false;
    }
    hFont = CreateFont(
        16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"Consolas"
    );
    if (!hFont) {
        return false;
    }

    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            HWND hwnd = CreateWindowEx(
                WS_EX_CLIENTEDGE,
                L"RICHEDIT50W",
                L"",
                WS_CHILD | WS_VISIBLE |
                ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
                0, 0, 0, 0,
                hwndParent,
                (HMENU)(INT_PTR)(IDC_MAIN_EDIT + r * kCols + c),
                hInstance,
                (LPVOID)nullptr
            );
            if (!hwnd) {
                for (int rr = 0; rr <= r; ++rr) {
                    for (int cc = 0; cc < (rr == r ? c : kCols); ++cc) {
                        if (cellEdits[rr][cc]) {
                            DestroyWindow(cellEdits[rr][cc]);
                            cellEdits[rr][cc] = nullptr;
                        }
                    }
                }
                DeleteObject(hFont);
                hFont = nullptr;
                return false;
            }
            SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, TRUE);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
            WNDPROC orig = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)EditProc);
            if (!orig) {
                for (int rr = 0; rr <= r; ++rr) {
                    for (int cc = 0; cc < (rr == r ? c + 1 : kCols); ++cc) {
                        if (cellEdits[rr][cc]) {
                            DestroyWindow(cellEdits[rr][cc]);
                            cellEdits[rr][cc] = nullptr;
                        }
                    }
                }
                DeleteObject(hFont);
                hFont = nullptr;
                return false;
            }
            SetOriginalProc(hwnd, orig);
            // default format
            CHARFORMAT2 cf = {};
            cf.cbSize = sizeof(cf);
            cf.dwMask = CFM_FACE | CFM_SIZE | CFM_WEIGHT;
            cf.yHeight = 16 * 20;
            wcscpy_s(cf.szFaceName, LF_FACESIZE, L"Consolas");
            SendMessage(hwnd, EM_SETCHARFORMAT, (WPARAM)SCF_ALL, (LPARAM)&cf);
            cellEdits[r][c] = hwnd;
        }
    }

    hwndVScroll = CreateWindowEx(0, L"SCROLLBAR", nullptr,
                                 WS_CHILD | WS_VISIBLE | SBS_VERT,
                                 0, 0, 0, 0, hwndParent, nullptr, hInstance, nullptr);
    if (!hwndVScroll) {
        return false;
    }

    Resize();
    return true;
}

void TextEditor::SetFontByMenuId(UINT id) {
    LOGFONT lf = {};
    lf.lfHeight = 16;
    lf.lfWeight = FW_NORMAL;
    wcscpy_s(lf.lfFaceName, LF_FACESIZE, L"Consolas");
    if (id == IDM_FONT_RASTER) {
        wcscpy_s(lf.lfFaceName, LF_FACESIZE, L"Terminal");
        lf.lfPitchAndFamily = DEFAULT_PITCH | FF_MODERN;
    } else if (id == IDM_FONT_VECTOR) {
        wcscpy_s(lf.lfFaceName, LF_FACESIZE, L"Arial");
        lf.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
    } else {
        wcscpy_s(lf.lfFaceName, LF_FACESIZE, L"Consolas");
        lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
    }

    // remember choice
    if (id == IDM_FONT_RASTER) currentFont = FontChoice::RasterTerminal;
    else if (id == IDM_FONT_VECTOR) currentFont = FontChoice::VectorArial;
    else currentFont = FontChoice::DefaultConsolas;
    ApplyCurrentFontToFocusedSelection();
}

void TextEditor::ApplyCurrentFontToFocusedSelection() {
    HWND target = GetFocus();
    if (!IsOurCell(target)) {
        return;
    }
    const wchar_t* face = L"Consolas";
    if (currentFont == FontChoice::RasterTerminal) face = L"Terminal";
    else if (currentFont == FontChoice::VectorArial) face = L"Arial";
    CHARFORMAT2 cf = {};
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_FACE | CFM_SIZE | CFM_WEIGHT;
    cf.yHeight = 16 * 20;
    wcscpy_s(cf.szFaceName, LF_FACESIZE, face);
    SendMessage(target, EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cf);
}

void TextEditor::ApplyFontToAll(const wchar_t* faceName, int heightLogical) {
    if (!faceName) {
        return;
    }
    HFONT newFont = CreateFont(
        heightLogical, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        faceName
    );
    if (!newFont) {
        return;
    }
    if (hFont) {
        DeleteObject(hFont);
    }
    hFont = newFont;
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (cellEdits[r][c]) {
                SendMessage(cellEdits[r][c], WM_SETFONT, (WPARAM)hFont, TRUE);
            }
        }
    }
    RelayoutGrid();
}

void TextEditor::SetFontPreset(FontPreset preset) {
    switch (preset) {
    case FontPreset::DefaultConsolas:
        ApplyFontToAll(L"Consolas", 16);
        break;
    case FontPreset::RasterTerminal:
        ApplyFontToAll(L"Terminal", 16);
        break;
    case FontPreset::VectorArial:
        ApplyFontToAll(L"Arial", 16);
        break;
    }
}

void TextEditor::Show() {
    bool any = false;
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (cellEdits[r][c]) {
                ShowWindow(cellEdits[r][c], SW_SHOW);
                any = true;
            }
        }
    }
    if (any) {
        isVisible = true;
    }
}

void TextEditor::Hide() {
    bool any = false;
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (cellEdits[r][c]) {
                ShowWindow(cellEdits[r][c], SW_HIDE);
                any = true;
            }
        }
    }
    if (any) {
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
    if (!cellEdits[0][0]) {
        return;
    }
    int row = 0;
    int col = 0;
    size_t pos = 0;
    size_t start = 0;
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (cellEdits[r][c]) {
                SetWindowText(cellEdits[r][c], L"");
                SendMessage(cellEdits[r][c], EM_SETMODIFY, FALSE, 0);
            }
        }
    }
    while (pos <= text.size() && row < kRows) {
        if (pos == text.size() || text[pos] == L'\t' || text[pos] == L'\n' || text[pos] == L'\r') {
            std::wstring cellText = text.substr(start, pos - start);
            if (cellEdits[row][col]) {
                SetWindowText(cellEdits[row][col], cellText.c_str());
            }
            if (pos < text.size() && text[pos] == L'\t') {
                ++col;
                if (col >= kCols) {
                    col = 0;
                    ++row;
                }
                ++pos;
            } else if (pos < text.size() && (text[pos] == L'\n' || text[pos] == L'\r')) {
                if (pos + 1 < text.size() && text[pos] == L'\r' && text[pos + 1] == L'\n') {
                    pos += 2;
                } else {
                    ++pos;
                }
                ++row;
                col = 0;
            } else {
                ++pos;
            }
            start = pos;
        } else {
            ++pos;
        }
    }
}

std::wstring TextEditor::GetText() const {
    std::wstring out;
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (!cellEdits[r][c]) {
                continue;
            }
            int len = GetWindowTextLength(cellEdits[r][c]);
            if (len < 0) {
                len = 0;
            }
            std::wstring cell(len + 1, L'\0');
            if (len > 0) {
                GetWindowText(cellEdits[r][c], &cell[0], len + 1);
                cell.resize(len);
            } else {
                cell.clear();
            }
            out += cell;
            if (c < kCols - 1) {
                out += L"\t";
            }
        }
        if (r < kRows - 1) {
            out += L"\r\n";
        }
    }
    return out;
}

void TextEditor::Clear() {
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (cellEdits[r][c]) {
                SetWindowText(cellEdits[r][c], L"");
                SendMessage(cellEdits[r][c], EM_SETMODIFY, FALSE, 0);
            }
        }
    }
}

void TextEditor::SetEditorFocus() {
    if (cellEdits[0][0]) {
        SetFocus(cellEdits[0][0]);
    }
}

void TextEditor::Resize() {
    RelayoutGrid();
}

LRESULT CALLBACK TextEditor::EditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    TextEditor* pThis = reinterpret_cast<TextEditor*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    WNDPROC orig = nullptr;
    if (pThis) {
        orig = pThis->GetOriginalProc(hwnd);
    }
    if (pThis && orig && pThis->IsOurCell(hwnd)) {
        switch (message) {
        case WM_CHAR:
            if (wParam == VK_ESCAPE) {
                pThis->Hide();
                return 0;
            }
            break;
        case WM_KEYUP:
        case WM_PASTE:
        case WM_CUT:
        case WM_UNDO:
        case WM_SETTEXT:
            pThis->RelayoutGrid();
            break;
        case WM_KEYDOWN:
            {
                bool ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
                if (ctrlPressed) {
                    switch (wParam) {
                    case 'A':
                        SendMessage(hwnd, EM_SETSEL, 0, -1);
                        return 0;
                    case 'O':
                        if (pThis->hwndParent) {
                            PostMessage(pThis->hwndParent, WM_COMMAND, IDM_FILE_OPEN, 0);
                        }
                        return 0;
                    case 'S':
                        if (pThis->hwndParent) {
                            PostMessage(pThis->hwndParent, WM_COMMAND, IDM_FILE_SAVE, 0);
                        }
                        return 0;
                    case 'W':
                    case 'Q':
                        if (pThis->hwndParent) {
                            SendMessage(pThis->hwndParent, WM_KEYDOWN, wParam, lParam);
                        }
                        return 0;
                    }
                }
                if (wParam == 'W' || wParam == 'A' || wParam == 'S' || wParam == 'D') {
                    if (pThis->hwndParent) {
                        SendMessage(pThis->hwndParent, WM_KEYDOWN, wParam, lParam);
                        return 0;
                    }
                }
            }
            break;
        }
        return CallWindowProc(orig, hwnd, message, wParam, lParam);
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

void TextEditor::Cut() {
    HWND target = GetFocus();
    if (!IsOurCell(target)) {
        target = cellEdits[0][0];
    }
    if (!target) {
        return;
    }
    int start = 0, end = 0;
    SendMessage(target, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
    if (start != end) {
        int len = GetWindowTextLength(target);
        if (len > 0) {
            std::wstring text(len + 1, L'\0');
            GetWindowText(target, &text[0], len + 1);
            text.resize(len);
            std::wstring selectedText = text.substr(start, end - start);
            if (ClipboardManager::CopyToClipboard(selectedText)) {
                SendMessage(target, EM_REPLACESEL, TRUE, (LPARAM)L"");
            }
        }
    }
}

void TextEditor::Copy() {
    HWND target = GetFocus();
    if (!IsOurCell(target)) {
        target = cellEdits[0][0];
    }
    if (!target) {
        return;
    }
    int start = 0, end = 0;
    SendMessage(target, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
    if (start != end) {
        int len = GetWindowTextLength(target);
        if (len > 0) {
            std::wstring text(len + 1, L'\0');
            GetWindowText(target, &text[0], len + 1);
            text.resize(len);
            std::wstring selectedText = text.substr(start, end - start);
            ClipboardManager::CopyToClipboard(selectedText);
        }
    }
}

void TextEditor::Paste() {
    HWND target = GetFocus();
    if (!IsOurCell(target)) {
        target = cellEdits[0][0];
    }
    if (!target) {
        return;
    }
    if (ClipboardManager::IsClipboardAvailable()) {
        std::wstring clipboardText = ClipboardManager::GetFromClipboard();
        if (!clipboardText.empty()) {
            SendMessage(target, EM_REPLACESEL, TRUE, (LPARAM)clipboardText.c_str());
        }
    }
}

void TextEditor::SelectAll() {
    HWND target = GetFocus();
    if (!IsOurCell(target)) {
        target = cellEdits[0][0];
    }
    if (!target) {
        return;
    }
    SendMessage(target, EM_SETSEL, 0, -1);
}

bool TextEditor::IsModified() const {
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (!cellEdits[r][c]) {
                continue;
            }
            LRESULT modified = SendMessage(cellEdits[r][c], EM_GETMODIFY, 0, 0);
            if (modified != 0) {
                return true;
            }
        }
    }
    return false;
}

void TextEditor::ResetModified() {
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (cellEdits[r][c]) {
                SendMessage(cellEdits[r][c], EM_SETMODIFY, FALSE, 0);
            }
        }
    }
}

WNDPROC TextEditor::GetOriginalProc(HWND hwnd) const {
    return (WNDPROC)(ULONG_PTR)GetProp(hwnd, L"TE_ORIGPROC");
}

bool TextEditor::SetOriginalProc(HWND hwnd, WNDPROC proc) {
    return SetProp(hwnd, L"TE_ORIGPROC", (HANDLE)(ULONG_PTR)proc) == TRUE;
}

bool TextEditor::IsOurCell(HWND hwnd) const {
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (cellEdits[r][c] == hwnd) {
                return true;
            }
        }
    }
    return false;
}

void TextEditor::RelayoutGrid() {
    if (!hwndParent) {
        return;
    }
    RECT rc;
    if (!GetClientRect(hwndParent, &rc)) {
        return;
    }
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;
    int margin = 10;
    int cellGap = 4;
    int gridLeft = margin;
    int gridTop = margin;
    int sbcx = GetSystemMetrics(SM_CXVSCROLL);
    int gridWidth = width - 2 * margin - sbcx;
    int gridHeight = height - 2 * margin;
    if (gridWidth <= 0 || gridHeight <= 0) {
        return;
    }
    int totalGapsW = (kCols - 1) * cellGap;
    int colWidth = (gridWidth - totalGapsW) / kCols;
    if (colWidth < 10) {
        colWidth = 10;
    }

    HFONT useFont = hFont;
    int singleLineHeight = 18;
    int wrapBias = 8; // approximate fallback char width
    if (useFont) {
        HDC hdc = GetDC(hwndParent);
        if (hdc) {
            HFONT old = (HFONT)SelectObject(hdc, useFont);
            TEXTMETRIC tm;
            if (GetTextMetrics(hdc, &tm)) {
                singleLineHeight = tm.tmHeight + tm.tmExternalLeading + 6;
                wrapBias = tm.tmAveCharWidth > 0 ? tm.tmAveCharWidth : wrapBias;
            }
            SelectObject(hdc, old);
            ReleaseDC(hwndParent, hdc);
        }
    }

    int y = gridTop - scrollOffsetY;
    for (int r = 0; r < kRows; ++r) {
        int rowHeight = singleLineHeight;
        for (int c = 0; c < kCols; ++c) {
            if (!cellEdits[r][c]) {
                continue;
            }
            int effectiveWidth = colWidth - 8 - wrapBias;
            if (effectiveWidth < 1) {
                effectiveWidth = 1;
            }
            int required = CalculateCellTextHeight(cellEdits[r][c], effectiveWidth);
            if (required > rowHeight) {
                rowHeight = required;
            }
        }

        int x = gridLeft;
        for (int c = 0; c < kCols; ++c) {
            if (cellEdits[r][c]) {
                SetWindowPos(cellEdits[r][c], nullptr, x, y, colWidth, rowHeight, SWP_NOZORDER | SWP_NOACTIVATE);
            }
            x += colWidth + cellGap;
        }
        y += rowHeight + cellGap;
    }

    BOOL redrawn = RedrawWindow(hwndParent, nullptr, nullptr,
                                 RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW);
    if (!redrawn) {
        InvalidateRect(hwndParent, nullptr, TRUE);
        UpdateWindow(hwndParent);
    }

    contentHeight = y - gridTop + scrollOffsetY - cellGap;
    UpdateScrollBar(rc);
    if (hwndVScroll) {
        SetWindowPos(hwndVScroll, nullptr, width - margin - sbcx, margin, sbcx, gridHeight, SWP_NOZORDER | SWP_NOACTIVATE);
        ShowWindow(hwndVScroll, SW_SHOW);
    }
}

int TextEditor::CalculateCellTextHeight(HWND hwndCell, int availableWidth) const {
    if (!hwndCell || availableWidth <= 0) {
        return 18;
    }
    int len = GetWindowTextLength(hwndCell);
    std::wstring text(len + 1, L'\0');
    if (len > 0) {
        GetWindowText(hwndCell, &text[0], len + 1);
        text.resize(len);
    } else {
        text.clear();
    }
    if (text.empty()) {
        return 18;
    }
    HDC hdc = GetDC(hwndCell);
    if (!hdc) {
        return 18;
    }
    HFONT useFont = (HFONT)SendMessage(hwndCell, WM_GETFONT, 0, 0);
    HFONT old = nullptr;
    if (useFont) {
        old = (HFONT)SelectObject(hdc, useFont);
    }
    RECT calc = {0, 0, availableWidth, 0};
    UINT format = DT_WORDBREAK | DT_CALCRECT | DT_EDITCONTROL;
    DrawText(hdc, text.c_str(), (int)text.size(), &calc, format);
    if (old) {
        SelectObject(hdc, old);
    }
    ReleaseDC(hwndCell, hdc);
    int h = (calc.bottom - calc.top) + 10;
    if (h < 18) {
        h = 18;
    }
    return h;
}

void TextEditor::UpdateScrollBar(const RECT& clientRect) {
    if (!hwndVScroll) {
        return;
    }
    int height = clientRect.bottom - clientRect.top - 2 * 10;
    if (height < 0) {
        height = 0;
    }
    SCROLLINFO si = {};
    si.cbSize = sizeof(si);
    si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
    si.nMin = 0;
    si.nMax = contentHeight > 0 ? contentHeight : height;
    si.nPage = height;
    if (scrollOffsetY < 0) {
        scrollOffsetY = 0;
    }
    int maxOffset = (si.nMax - (int)si.nPage);
    if (maxOffset < 0) {
        maxOffset = 0;
    }
    if (scrollOffsetY > maxOffset) {
        scrollOffsetY = maxOffset;
    }
    si.nPos = scrollOffsetY;
    SetScrollInfo(hwndVScroll, SB_CTL, &si, TRUE);
}

void TextEditor::ScrollBy(int deltaY) {
    int newPos = scrollOffsetY + deltaY;
    RECT rc;
    if (!GetClientRect(hwndParent, &rc)) {
        return;
    }
    int height = rc.bottom - rc.top - 2 * 10;
    int maxOffset = contentHeight - height;
    if (maxOffset < 0) {
        maxOffset = 0;
    }
    if (newPos < 0) newPos = 0;
    if (newPos > maxOffset) newPos = maxOffset;
    if (newPos != scrollOffsetY) {
        scrollOffsetY = newPos;
        RelayoutGrid();
    }
}

void TextEditor::OnVScroll(WPARAM wParam, LPARAM lParam) {
    if (!hwndVScroll || (HWND)lParam != hwndVScroll) {
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
        RelayoutGrid();
    }
}
