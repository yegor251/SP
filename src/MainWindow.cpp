#include "MainWindow.h"
#include <tchar.h>
#include <commdlg.h>
#include <string>
#include "FileManager.h"
#include "ReplaceDialog.h"
#include "DllDialog.h"
#include "DllFunctionManager.h"

MainWindow::MainWindow(HINSTANCE hInst) 
    : hwndMain(nullptr), hInstance(hInst), darkScreen(nullptr), textEditor(nullptr), dllManager(nullptr) {
    lastMousePos.x = 0;
    lastMousePos.y = 0;
}

MainWindow::~MainWindow() {
    delete darkScreen;
    delete textEditor;
    if (dllManager) delete dllManager;
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
        textEditor = new TextEditor(hwndMain, hInstance);
        if (textEditor) {
            textEditor->Create();
            textEditor->Show();
        }
        darkScreen = new DarkScreen(hwndMain, textEditor);
        dllManager = new DllFunctionManager();
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
        case WM_VSCROLL:
            if (pThis->textEditor) {
                pThis->textEditor->OnVScroll(wParam, lParam);
                return 0;
            }
            break;
            
        case WM_MOUSEWHEEL:
            if (pThis->textEditor) {
                pThis->textEditor->OnMouseWheel(wParam, lParam);
                return 0;
            }
            break;

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
        case WM_CLOSE:
            {
                bool canClose = true;
                if (pThis->textEditor && pThis->textEditor->IsModified()) {
                    int res = FileManager::ConfirmSaveChanges(hwnd);
                    if (res == IDYES) {
                        std::wstring text = pThis->textEditor->GetText();
                        if (!pThis->currentFilePath.empty()) {
                            if (!FileManager::SaveTextFileToPath(pThis->currentFilePath, text)) {
                                return 0;
                            }
                        } else {
                            std::wstring path;
                            if (!FileManager::SaveTextFile(hwnd, text, path)) {
                                return 0;
                            }
                            pThis->currentFilePath = path;
                        }
                        pThis->textEditor->ResetModified();
                    } else if (res == IDCANCEL) {
                        return 0;
                    }
                }
            }
            break;
        }
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

void MainWindow::OnCreate() {
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
        
    case IDM_FILE_OPEN:
        if (textEditor) {
            std::wstring text;
            std::wstring path;
            if (FileManager::OpenTextFile(hwndMain, text, path)) {
                textEditor->SetText(text);
                currentFilePath = path;
                textEditor->ResetModified();
            }
        }
        break;

    case IDM_FILE_SAVE:
        if (textEditor) {
            std::wstring text = textEditor->GetText();
            if (!currentFilePath.empty()) {
                FileManager::SaveTextFileToPath(currentFilePath, text);
                textEditor->ResetModified();
            } else {
                std::wstring path;
                if (FileManager::SaveTextFile(hwndMain, text, path)) {
                    currentFilePath = path;
                    textEditor->ResetModified();
                }
            }
        }
        break;

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

    case IDM_EDIT_REPLACE_DASHES:
        if (textEditor && textEditor->GetCellManager()) {
            ReplaceDialog::Show(hwndMain, [this](const std::wstring& target){
                if (this->textEditor && this->textEditor->GetCellManager()) {
                    this->textEditor->GetCellManager()->ReplaceWithDashesForAllCells(target);
                }
            });
        }
        break;
        
    case IDM_DLL_MENU:
        if (dllManager) {
            auto runDialog = [this]() {
                DllDialog::Show(hwndMain, *dllManager,
                    [this](int idx) {
                        if (textEditor && textEditor->GetCellManager()) {
                            const auto* item = dllManager->GetItem(idx);
                            if (item && item->proc) {
                                std::wstring input = textEditor->GetCellManager()->GetText();
                                wchar_t buf[65536] = { 0 };
                                item->proc(input.c_str(), buf, 65536);
                                std::wstring out = buf;
                                textEditor->GetCellManager()->SetText(out);
                                textEditor->GetCellManager()->ApplySelectedFontToAllCells();
                            }
                        }
                    },
                    [this](const std::wstring& path) {
                        if (dllManager->AddLibrary(path)) {}
                        PostMessage(hwndMain, WM_COMMAND, IDM_DLL_MENU, 0);
                    });
            };
            runDialog();
        }
        break;
        
        
    case IDM_HELP_ABOUT:
        AboutDialog::ShowDialog(hwndMain);
        break;
    case IDM_FONT_DEFAULT:
    case IDM_FONT_RASTER:
    case IDM_FONT_VECTOR:
        if (textEditor) {
            textEditor->SetFontByMenuId(LOWORD(wParam));
        }
        break;
    case WM_INITMENUPOPUP:
        break;
    }
}

void MainWindow::OnMouseMove(int x, int y) {
    lastMousePos.x = x;
    lastMousePos.y = y;
}

void MainWindow::OnKeyDown(WPARAM wParam) {
    if (darkScreen) {
        if (GetKeyState(VK_CONTROL) & 0x8000) {
            if (wParam == 'W') {
                darkScreen->StartScreensaver();
                return;
            } else if (wParam == 'Q') {
                darkScreen->StopScreensaver();
                return;
            }
        }
        
        if (darkScreen->IsActive()) {
            darkScreen->OnKeyDown(wParam);
        }
    }
}

void MainWindow::OnLeftMouseClick(int x, int y) {
    if (textEditor && textEditor->IsVisible()) {
        textEditor->SetEditorFocus();
    }
}

void MainWindow::OnMouseClick() {
}

void MainWindow::OnTimer(WPARAM wParam) {
}

void MainWindow::OnSize() {
    if (textEditor) {
        textEditor->Resize();
    }
}

