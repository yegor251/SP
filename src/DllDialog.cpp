#include "DllDialog.h"
#include <vector>
#include <string>
#include <commdlg.h>

static HWND g_hwndDialog = nullptr;
static HWND g_lastParent = nullptr;
static DllFunctionManager* g_manager = nullptr;
static DllDialog::ExecuteCallback g_onExecute;
static std::function<void(const std::wstring&)> g_onAddDll;
static std::vector<HWND> g_dllButtons;

static void ShowInfo(HWND parent, const wchar_t* msg, const wchar_t* cap = L"Info") {
    MessageBoxW(parent, msg, cap, MB_OK | MB_ICONINFORMATION);
}
static void ShowError(HWND parent, const wchar_t* msg, const wchar_t* cap = L"Error") {
    MessageBoxW(parent, msg, cap, MB_OK | MB_ICONERROR);
}

static LRESULT CALLBACK DllDlgWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        if (!g_manager) break;
        int y = 20;
        g_dllButtons.clear();
        const auto& items = g_manager->GetItems();
        for (size_t i = 0; i < items.size(); ++i) {
            HWND btn = CreateWindowEx(0, L"BUTTON", items[i].dllName.c_str(), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                20, y, 250, 30, hwnd, (HMENU)(1000 + i), GetModuleHandle(nullptr), nullptr);
            g_dllButtons.push_back(btn);
            y += 40;
        }
        HWND addBtn = CreateWindowEx(0, L"BUTTON", L"Add DLL...", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            20, y, 250, 30, hwnd, (HMENU)9000, GetModuleHandle(nullptr), nullptr);
        (void)addBtn;
        break;
    }
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id >= 1000 && id < 1000 + (int)g_dllButtons.size()) {
            // При нажатии на библиотеку: вызов обработчика (передают ошибки наружу)
            bool ok = true;
            try {
                if (g_onExecute) g_onExecute(id - 1000);
            } catch (...) {
                ok = false;
            }
            if (!ok) {
                ShowError(hwnd, L"Failed to execute function in the DLL");
                return 0;
            }
            DestroyWindow(hwnd);
            return 0;
        }
        if (id == 9000) { // Add DLL
            if (g_onAddDll) {
                OPENFILENAME ofn = { sizeof(ofn) };
                wchar_t szFile[MAX_PATH] = L"";
                ofn.hwndOwner = hwnd;
                ofn.lpstrFilter = L"DLL Files\0*.dll\0All Files\0*.*\0";
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = MAX_PATH;
                ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
                if (GetOpenFileName(&ofn)) {
                    bool result = false;
                    if (g_manager && g_manager->AddLibrary(szFile)) {
                        ShowInfo(hwnd, L"DLL loaded successfully");
                        result = true;
                    } else {
                        ShowError(hwnd, L"Failed to load DLL (already loaded, or error)");
                    }
                    // после успешного добавления пересоздать окно
                    if (result) {
                        DestroyWindow(hwnd);
                        return 0;
                    }
                    // иначе не закрываем окно
                    return 0;
                }
            }
            return 0;
        }
        break;
    }
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        g_hwndDialog = nullptr;
        break;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

void DllDialog::Show(HWND hwndParent, DllFunctionManager& manager, ExecuteCallback onExecute, std::function<void(const std::wstring&)> onAddDll) {
    if (g_hwndDialog) {
        SetForegroundWindow(g_hwndDialog);
        return;
    }
    g_manager = &manager;
    g_onExecute = onExecute;
    g_onAddDll = onAddDll;
    g_lastParent = hwndParent;
    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = DllDlgWndProc;
    wcex.hInstance = GetModuleHandle(nullptr);
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"DllDlgWndClass";
    wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    RegisterClassEx(&wcex);

    HWND hwndDialog = CreateWindowEx(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST, L"DllDlgWndClass", L"DLL manager",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME | DS_CENTER,
        0, 0, 300, 400, hwndParent, nullptr, GetModuleHandle(nullptr), nullptr
    );
    if (hwndDialog) {
        g_hwndDialog = hwndDialog;
        RECT rect;
        GetWindowRect(hwndDialog, &rect);
        int x = (GetSystemMetrics(SM_CXSCREEN) - (rect.right - rect.left)) / 2;
        int y = (GetSystemMetrics(SM_CYSCREEN) - (rect.bottom - rect.top)) / 2;
        SetWindowPos(hwndDialog, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        ShowWindow(hwndDialog, SW_SHOW);
        UpdateWindow(hwndDialog);
        EnableWindow(hwndParent, FALSE);
        MSG msg;
        while (IsWindow(hwndDialog) && GetMessage(&msg, nullptr, 0, 0)) {
            if (!IsDialogMessage(hwndDialog, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        EnableWindow(hwndParent, TRUE);
        SetForegroundWindow(hwndParent);
        // если г_hwndDialog стало nullptr — всё ок;
        // если hwndDialog уничтожен после успешной загрузки — пересоздаём выводом окна (в MainWindow тоже можно вызвать повторно Show, если хочется живое обновление)
    }
}
