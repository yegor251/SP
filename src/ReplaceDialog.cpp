#include "ReplaceDialog.h"

static HWND g_replaceDialog = nullptr;
static HWND g_editTarget = nullptr;
static bool g_replaceClosed = false;
static ReplaceDialog::ExecuteCallback g_onExecute;

static LRESULT CALLBACK ReplaceDlgWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        g_editTarget = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            L"EDIT",
            L"",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            20, 50, 260, 24,
            hwnd,
            (HMENU)100,
            GetModuleHandle(nullptr),
            nullptr
        );
        CreateWindowEx(0, L"STATIC", L"Enter target string:", WS_CHILD | WS_VISIBLE,
                        20, 20, 260, 20, hwnd, nullptr, GetModuleHandle(nullptr), nullptr);
        CreateWindowEx(0, L"BUTTON", L"Execute", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                        80, 90, 80, 26, hwnd, (HMENU)IDOK, GetModuleHandle(nullptr), nullptr);
        CreateWindowEx(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                        170, 90, 80, 26, hwnd, (HMENU)IDCANCEL, GetModuleHandle(nullptr), nullptr);
        return 0;
    }
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case IDOK: {
            if (g_editTarget && g_onExecute) {
                int len = GetWindowTextLength(g_editTarget);
                if (len < 0) len = 0;
                std::wstring buf(len + 1, L'\0');
                if (len > 0) {
                    int copied = GetWindowText(g_editTarget, &buf[0], len + 1);
                    if (copied < 0) copied = 0;
                    buf.resize(static_cast<size_t>(copied));
                } else {
                    buf.clear();
                }
                g_onExecute(buf);
            }
            g_replaceClosed = true;
            return 0;
        }
        case IDCANCEL:
            g_replaceClosed = true;
            return 0;
        }
        break;
    }
    case WM_CLOSE:
        g_replaceClosed = true;
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

void ReplaceDialog::Show(HWND hwndParent, ExecuteCallback onExecute) {
    g_onExecute = onExecute;
    g_replaceClosed = false;
    g_replaceDialog = nullptr;

    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = ReplaceDlgWndProc;
    wcex.hInstance = GetModuleHandle(nullptr);
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"ReplaceDialogClass";
    wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    RegisterClassEx(&wcex);

    HWND hwndDialog = CreateWindowEx(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        L"ReplaceDialogClass",
        L"Replace with dashes",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME | DS_CENTER,
        0, 0, 300, 150,
        hwndParent,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );

    if (hwndDialog) {
        g_replaceDialog = hwndDialog;

        RECT rect;
        GetWindowRect(hwndDialog, &rect);
        int x = (GetSystemMetrics(SM_CXSCREEN) - (rect.right - rect.left)) / 2;
        int y = (GetSystemMetrics(SM_CYSCREEN) - (rect.bottom - rect.top)) / 2;
        SetWindowPos(hwndDialog, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        ShowWindow(hwndDialog, SW_SHOW);
        UpdateWindow(hwndDialog);

        EnableWindow(hwndParent, FALSE);

        MSG msg;
        while (!g_replaceClosed && GetMessage(&msg, nullptr, 0, 0)) {
            if (msg.hwnd == hwndDialog || IsChild(hwndDialog, msg.hwnd)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        EnableWindow(hwndParent, TRUE);
        SetForegroundWindow(hwndParent);
        DestroyWindow(hwndDialog);
    }
}


