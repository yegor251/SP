#include "FileManager.h"
#include <commdlg.h>
#include <windowsx.h>
#include <vector>
#include <string>

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
        if (size.QuadPart > 64LL * 1024 * 1024) {
            CloseHandle(hFile);
            return false;
        }

        DWORD toRead = static_cast<DWORD>(size.QuadPart);
        std::vector<BYTE> bytes;
        bytes.resize(toRead);
        DWORD bytesRead = 0;
        BOOL ok = ReadFile(hFile, bytes.data(), toRead, &bytesRead, nullptr);
        CloseHandle(hFile);
        if (!ok) {
            return false;
        }
        bytes.resize(bytesRead);

        if (bytesRead == 0) {
            outText.clear();
            return true;
        }

        std::wstring textW;

        // Проверка BOM и преобразование
        if (bytesRead >= 3 && bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF) {
            // UTF-8 с BOM (Byte Order Mark: EF BB BF)
            // Пропускаем первые 3 байта BOM и конвертируем UTF-8 в UTF-16
            int wlen = MultiByteToWideChar(CP_UTF8, 0, 
                reinterpret_cast<LPCCH>(bytes.data() + 3), bytesRead - 3, nullptr, 0);
            if (wlen > 0) {
                textW.resize(wlen);
                MultiByteToWideChar(CP_UTF8, 0, 
                    reinterpret_cast<LPCCH>(bytes.data() + 3), bytesRead - 3, 
                    &textW[0], wlen);
            }
        }
        else if (bytesRead >= 2 && bytes[0] == 0xFF && bytes[1] == 0xFE) {
            // UTF-16 LE (Little Endian) с BOM (FF FE)
            // Пропускаем BOM и копируем UTF-16 данные напрямую
            size_t wcharCount = (bytesRead - 2) / sizeof(WCHAR);
            textW.assign(reinterpret_cast<const wchar_t*>(bytes.data() + 2), wcharCount);
        }
        else if (bytesRead >= 2 && bytes[1] == 0x00 && bytes[0] != 0x00) {
            // UTF-16 LE без BOM (первый байт не 0, второй байт 0)
            // Копируем UTF-16 данные напрямую
            size_t wcharCount = bytesRead / sizeof(WCHAR);
            textW.assign(reinterpret_cast<const wchar_t*>(bytes.data()), wcharCount);
        }
        else {
            // Попытка определить кодировку без BOM
            // Сначала пробуем UTF-8 без BOM
            int wlen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, 
                reinterpret_cast<LPCCH>(bytes.data()), bytesRead, nullptr, 0);
            
            if (wlen > 0 && GetLastError() != ERROR_NO_UNICODE_TRANSLATION) {
                // UTF-8 без BOM - успешно определили
                textW.resize(wlen);
                MultiByteToWideChar(CP_UTF8, 0, 
                    reinterpret_cast<LPCCH>(bytes.data()), bytesRead, 
                    &textW[0], wlen);
            }
            else {
                // Если UTF-8 не подошел, пробуем системную кодировку (ANSI/CP1251)
                wlen = MultiByteToWideChar(CP_ACP, 0, 
                    reinterpret_cast<LPCCH>(bytes.data()), bytesRead, nullptr, 0);
                if (wlen > 0) {
                    textW.resize(wlen);
                    MultiByteToWideChar(CP_ACP, 0, 
                        reinterpret_cast<LPCCH>(bytes.data()), bytesRead, 
                        &textW[0], wlen);
                }
            }
        }

        if (!textW.empty() && textW[0] == 0xFEFF) {
            textW.erase(textW.begin());
        }

        std::wstring normalized;
        normalized.reserve(textW.size());
        for (size_t i = 0; i < textW.size(); ++i) {
            WCHAR ch = textW[i];
            if (ch == L'\r') {
                if (i + 1 < textW.size() && textW[i + 1] == L'\n') {
                    ++i;
                }
                normalized.push_back(L'\n');
            } else {
                normalized.push_back(ch);
            }
        }

        std::wstring result;
        std::wstring current;
        int consecutiveNewlines = 0;
        for (size_t i = 0; i < normalized.size(); ++i) {
            WCHAR ch = normalized[i];
            if (ch == L'\n') {
                ++consecutiveNewlines;
                if (consecutiveNewlines >= 2) {
                    if (!result.empty()) {
                        result.push_back(L'\t');
                    }
                    while (!current.empty() && current.back() == L' ') {
                        current.pop_back();
                    }
                    result += current;
                    current.clear();
                    consecutiveNewlines = 0;
                } else {
                    current.push_back(L' ');
                }
            } else {
                consecutiveNewlines = 0;
                current.push_back(ch);
            }
        }
        
        if (!current.empty()) {
            if (!result.empty()) {
                result.push_back(L'\t');
            }
            result += current;
        }

        outText = result;
        return true;
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
    
        std::wstring toWrite = text;
        size_t pos = 0;
        while ((pos = toWrite.find(L"\t", pos)) != std::wstring::npos) {
            toWrite.replace(pos, 1, L"\r\n\r\n");
            pos += 4;
        }
    
        BOOL success = FALSE;
        
        bool needsConversion = false;
        for (wchar_t c : toWrite) {
            if (c > 0x7F) {
                needsConversion = true;
                break;
            }
        }
    
        if (needsConversion) {
            const BYTE utf8_bom[] = { 0xEF, 0xBB, 0xBF };
            DWORD written = 0;
            WriteFile(hOut, utf8_bom, sizeof(utf8_bom), &written, nullptr);
            
            int utf8_len = WideCharToMultiByte(CP_UTF8, 0, toWrite.c_str(), -1, nullptr, 0, nullptr, nullptr);
            if (utf8_len > 0) {
                std::vector<char> utf8_buffer(utf8_len);
                WideCharToMultiByte(CP_UTF8, 0, toWrite.c_str(), -1, utf8_buffer.data(), utf8_len, nullptr, nullptr);
                
                DWORD bytesWritten = 0;
                success = WriteFile(hOut, utf8_buffer.data(), utf8_len - 1, &bytesWritten, nullptr);
            }
        } else {
            int ansi_len = WideCharToMultiByte(CP_ACP, 0, toWrite.c_str(), -1, nullptr, 0, nullptr, nullptr);
            if (ansi_len > 0) {
                std::vector<char> ansi_buffer(ansi_len);
                WideCharToMultiByte(CP_ACP, 0, toWrite.c_str(), -1, ansi_buffer.data(), ansi_len, nullptr, nullptr);
                
                DWORD bytesWritten = 0;
                success = WriteFile(hOut, ansi_buffer.data(), ansi_len - 1, &bytesWritten, nullptr);
            }
        }
    
        CloseHandle(hOut);
        return success;
    }

    bool SaveTextFileToPath(const std::wstring& path, const std::wstring& text) {
        HANDLE hOut = CreateFile(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hOut == INVALID_HANDLE_VALUE) {
            return false;
        }
        std::wstring toWrite = text;
        size_t pos = 0;
        while ((pos = toWrite.find(L"\t", pos)) != std::wstring::npos) {
            toWrite.replace(pos, 1, L"\r\n\r\n");
            pos += 4;
        }
        const WCHAR bom = 0xFEFF;
        DWORD written = 0;
        BOOL ok1 = WriteFile(hOut, &bom, sizeof(bom), &written, nullptr);
        if (!ok1 || written != sizeof(bom)) {
            CloseHandle(hOut);
            return false;
        }
        const WCHAR* data = toWrite.c_str();
        DWORD bytesToWrite = static_cast<DWORD>(toWrite.size() * sizeof(WCHAR));
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


