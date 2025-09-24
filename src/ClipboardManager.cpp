#include "ClipboardManager.h"
#include <tchar.h>

bool ClipboardManager::CopyToClipboard(const std::wstring& text) {
    if (text.empty()) {
        return false;
    }

    if (!OpenClipboard(nullptr)) {
        return false;
    }

    EmptyClipboard();

    size_t textSize = (text.length() + 1) * sizeof(wchar_t);
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, textSize);
    
    if (!hGlobal) {
        CloseClipboard();
        return false;
    }

    wchar_t* pGlobal = static_cast<wchar_t*>(GlobalLock(hGlobal));
    if (!pGlobal) {
        GlobalFree(hGlobal);
        CloseClipboard();
        return false;
    }

    wcscpy_s(pGlobal, text.length() + 1, text.c_str());
    GlobalUnlock(hGlobal);

    SetClipboardData(CF_UNICODETEXT, hGlobal);
    CloseClipboard();

    return true;
}

std::wstring ClipboardManager::GetFromClipboard() {
    if (!IsClipboardAvailable()) {
        return L"";
    }

    if (!OpenClipboard(nullptr)) {
        return L"";
    }

    HGLOBAL hGlobal = GetClipboardData(CF_UNICODETEXT);
    if (!hGlobal) {
        CloseClipboard();
        return L"";
    }

    wchar_t* pGlobal = static_cast<wchar_t*>(GlobalLock(hGlobal));
    if (!pGlobal) {
        CloseClipboard();
        return L"";
    }

    std::wstring result(pGlobal);
    GlobalUnlock(hGlobal);
    CloseClipboard();

    return result;
}

bool ClipboardManager::IsClipboardAvailable() {
    return IsClipboardFormatAvailable(CF_UNICODETEXT) != 0;
}

void ClipboardManager::ClearClipboard() {
    if (OpenClipboard(nullptr)) {
        EmptyClipboard();
        CloseClipboard();
    }
}
