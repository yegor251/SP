#include <windows.h>
#include <tchar.h>
#include "MainWindow.h"

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                       _In_opt_ HINSTANCE hPrevInstance,
                       _In_ LPTSTR    lpCmdLine,
                       _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    MainWindow mainWindow(hInstance);
    
    if (!mainWindow.Create()) {
        MessageBox(nullptr, L"Error creating window!", L"Error", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    mainWindow.Show(nCmdShow);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}