// ed.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "ed.h"
#include "Scintilla.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
//WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
//WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    // https://www.scintilla.org/Steps.html
    HWND hwndParent = nullptr;
    HMODULE hmod = LoadLibrary(L"SciLexer.DLL");
    if (hmod == NULL)
    {
      MessageBox(hwndParent,
        L"The Scintilla DLL could not be loaded.",
        L"Error loading Scintilla",
        MB_OK | MB_ICONERROR);
    }

    // Initialize global strings
//    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
//    LoadStringW(hInstance, IDC_ED, szWindowClass, MAX_LOADSTRING);
//    MyRegisterClass(hInstance);

    HWND hwndScintilla = CreateWindowW(L"Scintilla", L"r-e-a-d (C) 2020 EkwoTECH GmbH Friedrichshafen", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
    if (!hwndScintilla)
    {
      return FALSE;
    }
    ShowWindow(hwndScintilla, nCmdShow);
    UpdateWindow(hwndScintilla);

// example:   SCI_SETTEXT(<unused>, const char *text)
    char texti[72] = "Hallo Tailchen, wie geht es Dir?\nDanke, gut!\nUnd Dir?\nDanke, mir auch.";
//  (A) fast version:
/*    int(*fn)(void*, int, int, int);
    void * ptr;
    int settext;
    int *tmp = reinterpret_cast<int*>(texti);

    fn = (int(__cdecl *)(void *, int, int, int))SendMessage(hwndScintilla, SCI_GETDIRECTFUNCTION, 0, 0);
    ptr = (void *)SendMessage(hwndScintilla, SCI_GETDIRECTPOINTER, 0, 0);

    settext = fn(ptr, SCI_SETTEXT, 0, int(texti));
 */
    // (B) regular (slow?) version
    SendMessage(hwndScintilla, SCI_SETTEXT, 0, int(texti));


    // (i) define styles
    SendMessage(hwndScintilla, SCI_STYLESETFORE, 0, 0x0040FF);
    SendMessage(hwndScintilla, SCI_STYLESETBACK, 0, 0xBBBBBB);
    SendMessage(hwndScintilla, SCI_STYLESETBOLD, 0, true);

    SendMessage(hwndScintilla, SCI_STYLESETFORE, 1, 0xff40FF);
    SendMessage(hwndScintilla, SCI_STYLESETBACK, 1, 0x3e3e3e);

    SendMessage(hwndScintilla, SCI_STYLESETFORE, 2, 0xFF0000); // bgr
    SendMessage(hwndScintilla, SCI_STYLESETBACK, 2, 0xDDDDDD);


    SendMessage(hwndScintilla, SCI_STARTSTYLING, 0, 1); // SCI_STARTSTYLING(position start, int unused)
    // (ii) now just concatenate colored/styled sections
    int length = 5;
    SendMessage(hwndScintilla, SCI_SETSTYLING, length, 2); // SCI_SETSTYLING(position length, int style)
    SendMessage(hwndScintilla, SCI_SETSTYLING, 11, 1); // SCI_SETSTYLING(position length, int style)
    SendMessage(hwndScintilla, SCI_SETSTYLING, 4, 2); // SCI_SETSTYLING(position length, int style)
    SendMessage(hwndScintilla, SCI_SETSTYLING, 13, 1); // \n counts as 1 length
    SendMessage(hwndScintilla, SCI_SETSTYLING, 12, 0); // SCI_SETSTYLING(position length, int style)
    SendMessage(hwndScintilla, SCI_SETSTYLING, 9, 2); // SCI_SETSTYLING(position length, int style)

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ED));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


/*
//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ED));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_ED);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}
*/
//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
