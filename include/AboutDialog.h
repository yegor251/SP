#pragma once
#include <windows.h>
#include "Resource.h"

class AboutDialog {
public:
    static INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static void ShowDialog(HWND hwndParent);
};
