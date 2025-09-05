#include "AboutDialog.h"

static HWND g_hwndDialog = nullptr;
static bool g_dialogClosed = false;

static LRESULT CALLBACK DialogWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            g_dialogClosed = true;
            return 0;
        }
        break;
        
    case WM_CLOSE:
        g_dialogClosed = true;
        return 0;
    }
    
    return DefWindowProc(hwnd, message, wParam, lParam);
}

INT_PTR CALLBACK AboutDialog::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG:
        return TRUE;
        
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hwndDlg, LOWORD(wParam));
            return TRUE;
        }
        break;
        
    case WM_CLOSE:
        EndDialog(hwndDlg, IDCANCEL);
        return TRUE;
    }
    return FALSE;
}

void AboutDialog::ShowDialog(HWND hwndParent) {
    g_dialogClosed = false;
    g_hwndDialog = nullptr;
    
    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = DialogWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = GetModuleHandle(nullptr);
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"AboutDialogClass";
    wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    
    RegisterClassEx(&wcex);
    
    HWND hwndDialog = CreateWindowEx(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        L"AboutDialogClass",
        L"About",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME | DS_CENTER,
        0, 0, 300, 200,
        hwndParent,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );
    
    if (hwndDialog) {
        g_hwndDialog = hwndDialog;
        
        HWND hwndStatic = CreateWindowEx(
            0,
            L"STATIC",
            L"Simple WinAPI Application\n\n"
            L"using Windows API",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            20, 20, 260, 100,
            hwndDialog,
            (HMENU)IDC_STATIC_ABOUT,
            GetModuleHandle(nullptr),
            nullptr
        );
        
        HWND hwndOK = CreateWindowEx(
            0,
            L"BUTTON",
            L"OK",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            120, 140, 60, 25,
            hwndDialog,
            (HMENU)IDOK,
            GetModuleHandle(nullptr),
            nullptr
        );
        
        RECT rect;
        GetWindowRect(hwndDialog, &rect);
        int x = (GetSystemMetrics(SM_CXSCREEN) - (rect.right - rect.left)) / 2;
        int y = (GetSystemMetrics(SM_CYSCREEN) - (rect.bottom - rect.top)) / 2;
        SetWindowPos(hwndDialog, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        
        ShowWindow(hwndDialog, SW_SHOW);
        UpdateWindow(hwndDialog);
        
        EnableWindow(hwndParent, FALSE);
        
        MSG msg;
        while (!g_dialogClosed && GetMessage(&msg, nullptr, 0, 0)) {
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
