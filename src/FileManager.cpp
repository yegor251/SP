#include "FileManager.h"
#include <commdlg.h>
#include <windowsx.h>

namespace FileManager {
    bool OpenTextFile(HWND owner, std::wstring& outText, std::wstring& outPath) {
        OPENFILENAME ofn = {};
        WCHAR szFile[MAX_PATH] = {0};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = owner;
        ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
        ofn.lpstrDefExt = L"txt";
        if (!GetOpenFileName(&ofn)) {
            return false;
        }
        outPath = szFile;

        HANDLE hFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            return false;
        }

        LARGE_INTEGER size;
        if (!GetFileSizeEx(hFile, &size)) {
            CloseHandle(hFile);
            return false;
        }
        if (size.QuadPart > 20 * 1024 * 1024) {
            CloseHandle(hFile);
            return false;
        }

        DWORD toRead = static_cast<DWORD>(size.QuadPart);
        if (toRead == 0) {
            outText.clear();
            CloseHandle(hFile);
            return true;
        }

        std::wstring buffer;
        buffer.resize((toRead / sizeof(WCHAR)) + 2);
        DWORD bytesRead = 0;
        BOOL ok = ReadFile(hFile, &buffer[0], toRead, &bytesRead, nullptr);
        CloseHandle(hFile);
        if (!ok) {
            return false;
        }

        if (bytesRead % 2 == 0) {
            size_t wcharCount = bytesRead / sizeof(WCHAR);
            buffer.resize(wcharCount);
            if (!buffer.empty() && buffer[0] == 0xFEFF) {
                outText = buffer.substr(1);
            } else {
                outText = buffer;
            }
            return true;
        } else {
            std::wstring wide;
            wide.resize(bytesRead);
            for (DWORD i = 0; i < bytesRead; ++i) wide[i] = (WCHAR)((unsigned char)buffer[i]);
            outText = wide;
            return true;
        }
    }

    bool SaveTextFile(HWND owner, const std::wstring& text, std::wstring& outPath) {
        OPENFILENAME sfn = {};
        WCHAR szFileOut[MAX_PATH] = {0};
        sfn.lStructSize = sizeof(sfn);
        sfn.hwndOwner = owner;
        sfn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
        sfn.lpstrFile = szFileOut;
        sfn.nMaxFile = MAX_PATH;
        sfn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
        sfn.lpstrDefExt = L"txt";
        if (!GetSaveFileName(&sfn)) {
            return false;
        }
        outPath = szFileOut;

        HANDLE hOut = CreateFile(szFileOut, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hOut == INVALID_HANDLE_VALUE) {
            return false;
        }

        const WCHAR bom = 0xFEFF;
        DWORD written = 0;
        BOOL ok1 = WriteFile(hOut, &bom, sizeof(bom), &written, nullptr);
        if (!ok1 || written != sizeof(bom)) {
            CloseHandle(hOut);
            return false;
        }
        const WCHAR* data = text.c_str();
        DWORD bytesToWrite = static_cast<DWORD>(text.size() * sizeof(WCHAR));
        DWORD bytesWritten = 0;
        BOOL ok2 = WriteFile(hOut, data, bytesToWrite, &bytesWritten, nullptr);
        CloseHandle(hOut);
        return ok2 && bytesWritten == bytesToWrite;
    }

    bool SaveTextFileToPath(const std::wstring& path, const std::wstring& text) {
        HANDLE hOut = CreateFile(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hOut == INVALID_HANDLE_VALUE) {
            return false;
        }
        const WCHAR bom = 0xFEFF;
        DWORD written = 0;
        BOOL ok1 = WriteFile(hOut, &bom, sizeof(bom), &written, nullptr);
        if (!ok1 || written != sizeof(bom)) {
            CloseHandle(hOut);
            return false;
        }
        const WCHAR* data = text.c_str();
        DWORD bytesToWrite = static_cast<DWORD>(text.size() * sizeof(WCHAR));
        DWORD bytesWritten = 0;
        BOOL ok2 = WriteFile(hOut, data, bytesToWrite, &bytesWritten, nullptr);
        CloseHandle(hOut);
        return ok2 && bytesWritten == bytesToWrite;
    }

    int ConfirmSaveChanges(HWND owner) {
        int r = MessageBox(owner, L"Save changes before exit?", L"Unsaved changes", MB_ICONQUESTION | MB_YESNOCANCEL | MB_APPLMODAL);
        return r;
    }
}


