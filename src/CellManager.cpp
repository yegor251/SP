#include "CellManager.h"
#include "Resource.h"
#include <tchar.h>
#include <string>
#include <Richedit.h>
#include <RichEdit.h>

CellManager::CellManager(HWND parent, HINSTANCE hInst) 
    : hwndParent(parent), hInstance(hInst), hMsftedit(nullptr), scrollOffset(0),
      contentChangedCallback(nullptr), contentChangedUserData(nullptr) {
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            cellEdits[r][c] = nullptr;
        }
        rowHeights[r] = kDefaultRowHeight;
    }
}

void CellManager::SetRowHeight(int row, int height) {
    if (row >= 0 && row < kRows) {
        rowHeights[row] = height;
    }
}

int CellManager::GetRowHeight(int row) const {
    if (row >= 0 && row < kRows) {
        return rowHeights[row];
    }
    return kDefaultRowHeight;
}

void CellManager::IncreaseRowHeight(int row, int increment) {
    if (row >= 0 && row < kRows) {
        rowHeights[row] += increment;
        RelayoutGrid();
    }
}

void CellManager::CheckAndAdjustRowHeight(int row) {
    if (row < 0 || row >= kRows) {
        return;
    }
    
    int maxHeight = kDefaultRowHeight;
    
    for (int c = 0; c < kCols; ++c) {
        if (cellEdits[row][c]) {
            RECT rect = {};
            SendMessage(cellEdits[row][c], EM_GETRECT, 0, (LPARAM)&rect);
            
            int lineCount = SendMessage(cellEdits[row][c], EM_GETLINECOUNT, 0, 0);
            if (lineCount > 0) {
                int textHeight = lineCount * kDefaultRowHeight;
                if (textHeight > maxHeight) {
                    maxHeight = textHeight;
                }
            }
        }
    }
    if (maxHeight != rowHeights[row]) {
        rowHeights[row] = maxHeight;
        RelayoutGrid();
    }
}

CellManager::~CellManager() {
    Destroy();
}

void CellManager::Destroy() {
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (cellEdits[r][c]) {
                DestroyWindow(cellEdits[r][c]);
                cellEdits[r][c] = nullptr;
            }
        }
    }
    if (hMsftedit) {
        FreeLibrary(hMsftedit);
        hMsftedit = nullptr;
    }
}

bool CellManager::Create() {
    if (!hwndParent || !hInstance) {
        return false;
    }

    hMsftedit = LoadLibrary(L"Msftedit.dll");
    if (!hMsftedit) {
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
                return false;
            }
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
                return false;
            }
            SetOriginalProc(hwnd, orig);
            CHARFORMAT2 cf = {};
            cf.cbSize = sizeof(cf);
            cf.dwMask = CFM_FACE | CFM_SIZE | CFM_WEIGHT;
            cf.yHeight = 16 * 20;
            wcscpy_s(cf.szFaceName, LF_FACESIZE, L"Consolas");
            SendMessage(hwnd, EM_SETCHARFORMAT, (WPARAM)SCF_ALL, (LPARAM)&cf);
            cellEdits[r][c] = hwnd;
        }
    }
    return true;
}

void CellManager::Show() {
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (cellEdits[r][c]) {
                ShowWindow(cellEdits[r][c], SW_SHOW);
            }
        }
    }
}

void CellManager::Hide() {
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (cellEdits[r][c]) {
                ShowWindow(cellEdits[r][c], SW_HIDE);
            }
        }
    }
}

void CellManager::SetVisible(bool visible) {
    if (visible) {
        Show();
    } else {
        Hide();
    }
}

bool CellManager::IsVisible() const {
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (cellEdits[r][c] && IsWindowVisible(cellEdits[r][c])) {
                return true;
            }
        }
    }
    return false;
}

void CellManager::SetText(const std::wstring& text) {
    if (!cellEdits[0][0]) {
        return;
    }
    
    // Сбрасываем все высоты к начальным
    for (int r = 0; r < kRows; ++r) {
        rowHeights[r] = kDefaultRowHeight;
    }
    
    int row = 0;
    int col = 0;
    size_t pos = 0;
    size_t start = 0;
    
    // Очищаем все ячейки
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
                    // При переходе на новую строку увеличиваем высоту
                    if (row < kRows) {
                        IncreaseRowHeight(row);
                    }
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
                // При переходе на новую строку увеличиваем высоту
                if (row < kRows) {
                    IncreaseRowHeight(row);
                }
            } else {
                ++pos;
            }
            start = pos;
        } else {
            ++pos;
        }
    }
}

std::wstring CellManager::GetText() const {
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

void CellManager::Clear() {
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (cellEdits[r][c]) {
                SetWindowText(cellEdits[r][c], L"");
                SendMessage(cellEdits[r][c], EM_SETMODIFY, FALSE, 0);
            }
        }
    }
}

void CellManager::SetEditorFocus() {
    if (cellEdits[0][0]) {
        SetFocus(cellEdits[0][0]);
    }
}

void CellManager::Resize() {
    RelayoutGrid();
}

bool CellManager::IsModified() const {
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

void CellManager::ResetModified() {
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (cellEdits[r][c]) {
                SendMessage(cellEdits[r][c], EM_SETMODIFY, FALSE, 0);
            }
        }
    }
}

HWND CellManager::GetHandle() const {
    return cellEdits[0][0];
}

bool CellManager::IsOurCell(HWND hwnd) const {
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (cellEdits[r][c] == hwnd) {
                return true;
            }
        }
    }
    return false;
}

void CellManager::SetFont(HFONT font) {
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (cellEdits[r][c]) {
                SendMessage(cellEdits[r][c], WM_SETFONT, (WPARAM)font, TRUE);
            }
        }
    }
}

void CellManager::SetCharFormat(const wchar_t* faceName, int height) {
    CHARFORMAT2 cf = {};
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_FACE | CFM_SIZE | CFM_WEIGHT;
    cf.yHeight = height * 20;
    wcscpy_s(cf.szFaceName, LF_FACESIZE, faceName);
    
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (cellEdits[r][c]) {
                SendMessage(cellEdits[r][c], EM_SETCHARFORMAT, (WPARAM)SCF_ALL, (LPARAM)&cf);
            }
        }
    }
}

void CellManager::RelayoutGrid() {
    RelayoutGridWithScrollOffset(scrollOffset);
}

void CellManager::RelayoutGridWithScrollOffset(int scrollOffset) {
    if (!hwndParent) {
        return;
    }
    this->scrollOffset = scrollOffset;
    RECT rc;
    if (!GetClientRect(hwndParent, &rc)) {
        return;
    }
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;
    int margin = 10;
    int cellGap = 4;
    int gridLeft = margin;
    int gridTop = margin - scrollOffset;
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

    int y = gridTop;
    for (int r = 0; r < kRows; ++r) {
        int rowHeight = rowHeights[r];

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
}

int CellManager::GetContentHeight() const {
    if (!hwndParent) {
        return 0;
    }
    RECT rc;
    if (!GetClientRect(hwndParent, &rc)) {
        return 0;
    }
    int margin = 10;
    int cellGap = 4;
    int gridTop = margin;

    int totalHeight = 0;
    for (int r = 0; r < kRows; ++r) {
        totalHeight += rowHeights[r];
        if (r < kRows - 1) {
            totalHeight += cellGap;
        }
    }
    
    return totalHeight + 2 * margin;
}

void CellManager::SetContentChangedCallback(ContentChangedCallback callback, void* userData) {
    contentChangedCallback = callback;
    contentChangedUserData = userData;
}

WNDPROC CellManager::GetOriginalProc(HWND hwnd) const {
    return (WNDPROC)(ULONG_PTR)GetProp(hwnd, L"CM_ORIGPROC");
}

bool CellManager::SetOriginalProc(HWND hwnd, WNDPROC proc) {
    return SetProp(hwnd, L"CM_ORIGPROC", (HANDLE)(ULONG_PTR)proc) == TRUE;
}

LRESULT CALLBACK CellManager::EditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    CellManager* pThis = reinterpret_cast<CellManager*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
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
            // При нажатии Enter увеличиваем высоту текущей строки
            else if (wParam == VK_RETURN) {
                // Находим строку текущей ячейки
                for (int r = 0; r < kRows; ++r) {
                    for (int c = 0; c < kCols; ++c) {
                        if (pThis->cellEdits[r][c] == hwnd) {
                            pThis->IncreaseRowHeight(r);
                            break;
                        }
                    }
                }
            }
            break;
        case WM_KEYUP:
        case WM_PASTE:
        case WM_CUT:
        case WM_UNDO:
        case WM_SETTEXT:
            // Находим строку текущей ячейки и проверяем высоту
            for (int r = 0; r < kRows; ++r) {
                for (int c = 0; c < kCols; ++c) {
                    if (pThis->cellEdits[r][c] == hwnd) {
                        pThis->CheckAndAdjustRowHeight(r);
                        break;
                    }
                }
            }
            if (pThis->contentChangedCallback) {
                pThis->contentChangedCallback(pThis->contentChangedUserData);
            }
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
