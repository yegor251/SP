#include "ClipboardHandler.h"
#include "ClipboardManager.h"
#include "CellManager.h"

ClipboardHandler::ClipboardHandler(HWND parent, CellManager* cellMgr) : hwndParent(parent), cellManager(cellMgr) {
}

void ClipboardHandler::Cut() {
    HWND target = GetTargetWindow();
    if (!target) {
        return;
    }
    std::wstring selectedText = GetSelectedText(target);
    if (!selectedText.empty() && ClipboardManager::CopyToClipboard(selectedText)) {
        SendMessage(target, EM_REPLACESEL, TRUE, (LPARAM)L"");
    }
}

void ClipboardHandler::Copy() {
    HWND target = GetTargetWindow();
    if (!target) {
        return;
    }
    std::wstring selectedText = GetSelectedText(target);
    if (!selectedText.empty()) {
        ClipboardManager::CopyToClipboard(selectedText);
    }
}

void ClipboardHandler::Paste() {
    HWND target = GetTargetWindow();
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

void ClipboardHandler::SelectAll() {
    HWND target = GetTargetWindow();
    if (!target) {
        return;
    }
    SendMessage(target, EM_SETSEL, 0, -1);
}

bool ClipboardHandler::IsClipboardAvailable() const {
    return ClipboardManager::IsClipboardAvailable();
}

std::wstring ClipboardHandler::GetFromClipboard() const {
    return ClipboardManager::GetFromClipboard();
}

bool ClipboardHandler::CopyToClipboard(const std::wstring& text) const {
    return ClipboardManager::CopyToClipboard(text);
}

HWND ClipboardHandler::GetTargetWindow() const {
    HWND target = GetFocus();
    if (!cellManager || !cellManager->IsOurCell(target)) {
        return nullptr;
    }
    return target;
}

std::wstring ClipboardHandler::GetSelectedText(HWND hwnd) const {
    int start = 0, end = 0;
    SendMessage(hwnd, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
    if (start == end) {
        return L"";
    }
    int len = GetWindowTextLength(hwnd);
    if (len <= 0) {
        return L"";
    }
    std::wstring text(len + 1, L'\0');
    GetWindowText(hwnd, &text[0], len + 1);
    text.resize(len);
    if (end > (int)text.size()) {
        end = (int)text.size();
    }
    if (start > (int)text.size()) {
        start = (int)text.size();
    }
    if (start >= end) {
        return L"";
    }
    return text.substr(start, end - start);
}
