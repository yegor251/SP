#include "MainWindow.h"
#include <tchar.h>

MainWindow::MainWindow(HINSTANCE hInst) 
    : hwndMain(nullptr), hInstance(hInst), darkScreen(nullptr), textEditor(nullptr) {
    lastMousePos.x = 0;
    lastMousePos.y = 0;
}

MainWindow::~MainWindow() {
    delete darkScreen;
    delete textEditor;
}

bool MainWindow::Create() {
    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"MainWindowClass";
    wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex)) {
        return false;
    }

    hwndMain = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        L"MainWindowClass",
        L"Simple WinAPI Application",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0,
        800, 600,
        nullptr,
        menuManager.CreateMainMenu(),
        hInstance,
        this
    );

    if (hwndMain) {
        darkScreen = new DarkScreen(hwndMain);
        textEditor = new TextEditor(hwndMain, hInstance);
        if (textEditor) {
            textEditor->Create();
            textEditor->Show();
        }
    }

    return hwndMain != nullptr;
}

void MainWindow::Show(int nCmdShow) {
    if (hwndMain) {
        ShowWindow(hwndMain, nCmdShow);
        UpdateWindow(hwndMain);
    }
}

LRESULT CALLBACK MainWindow::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    MainWindow* pThis = nullptr;
    
    if (message == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<MainWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        switch (message) {
        case WM_CREATE:
            pThis->OnCreate();
            return 0;

        case WM_PAINT:
            pThis->OnPaint();
            return 0;

        case WM_COMMAND:
            pThis->OnCommand(wParam);
            return 0;

        case WM_SIZE:
            pThis->OnSize();
            return 0;

        case WM_MOUSEMOVE:
            pThis->OnMouseMove(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_KEYDOWN:
            pThis->OnKeyDown(wParam);
            return 0;

        case WM_LBUTTONDOWN:
            pThis->OnLeftMouseClick(LOWORD(lParam), HIWORD(lParam));
            return 0;
            
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
            pThis->OnMouseClick();
            return 0;

        case WM_TIMER:
            pThis->OnTimer(wParam);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

void MainWindow::OnCreate() {
    if (darkScreen) {
        darkScreen->StartInactivityTimer();
    }
}

void MainWindow::OnPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwndMain, &ps);
    
    if (darkScreen && darkScreen->IsActive()) {
        darkScreen->OnPaint(hdc);
    } else {
        RECT rect;
        GetClientRect(hwndMain, &rect);
        
        FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
    }
    
    EndPaint(hwndMain, &ps);
}

void MainWindow::OnCommand(WPARAM wParam) {
    switch (LOWORD(wParam)) {
        
    case IDM_FILE_EXIT:
        PostMessage(hwndMain, WM_CLOSE, 0, 0);
        break;
        
    case IDM_EDIT_CUT:
        if (textEditor) {
            textEditor->Cut();
        }
        break;
        
    case IDM_EDIT_COPY:
        if (textEditor) {
            textEditor->Copy();
        }
        break;
        
    case IDM_EDIT_PASTE:
        if (textEditor) {
            textEditor->Paste();
        }
        break;
        
    case IDM_EDIT_SELECT_ALL:
        if (textEditor) {
            textEditor->SelectAll();
        }
        break;
        
        
    case IDM_HELP_ABOUT:
        AboutDialog::ShowDialog(hwndMain);
        break;
    }
}

void MainWindow::OnMouseMove(int x, int y) {
    if (x != lastMousePos.x || y != lastMousePos.y) {
        lastMousePos.x = x;
        lastMousePos.y = y;
        
        if (darkScreen) {
            darkScreen->OnUserActivity();
        }
    }
}

void MainWindow::OnKeyDown(WPARAM wParam) {
    if (darkScreen) {
        darkScreen->OnUserActivity();
    }
}

void MainWindow::OnLeftMouseClick(int x, int y) {
    if (darkScreen) {
        darkScreen->OnUserActivity();
    }
    
    if (textEditor && textEditor->IsVisible()) {
        textEditor->SetEditorFocus();
    }
}

void MainWindow::OnMouseClick() {
    if (darkScreen) {
        darkScreen->OnUserActivity();
    }
}

void MainWindow::OnTimer(WPARAM wParam) {
    if (darkScreen) {
        darkScreen->OnTimer(wParam);
    }
}

void MainWindow::OnSize() {
    if (textEditor) {
        textEditor->Resize();
    }
}

