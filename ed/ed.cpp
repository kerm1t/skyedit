// ed.cpp : Defines the entry point for the application.
// this should probably renamed to read.cpp or main.cpp

#include "stdafx.h"
#include "ed.h"
#include "Scintilla.h" // 2do: move into this repo
#include "read_and_parse.h"
#include <Commdlg.h>

high_text our_text;

HWND hwndScintilla;
OPENFILENAME ofn;
char szFile[260];
BOOL b_filechosen;
void color_scintilla();

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
//BOOL                InitInstance(HINSTANCE, int);

// Forward declarations of functions included in this code module:
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
    HMODULE hmod = LoadLibrary(L"SciLexer.DLL");
    if (hmod == NULL)
    {
      HWND hwndParent = nullptr;
      MessageBox(hwndParent,
        L"The Scintilla DLL could not be loaded.",
        L"Error loading Scintilla",
        MB_OK | MB_ICONERROR);
    }

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ED, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    hInst = hInstance; // !!

    // show window
    // 2do: add more controls, i.e. a listbox
    HWND hwndBase = CreateWindowW(szWindowClass, L"r-e-a-d (C) 2020 EkwoTECH GmbH, Friedrichshafen", WS_OVERLAPPEDWINDOW,
//    HWND hwndScintilla = CreateWindowW(L"Scintilla", L"r-e-a-d (C) 2020 EkwoTECH GmbH, Friedrichshafen", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
    if (!hwndBase)
    {
      return FALSE;
    }
    ShowWindow(hwndBase, nCmdShow);
    UpdateWindow(hwndBase);



    // now load text and parse
    // in: filename
    // out: text, list of positions (i.e. where std-text changes to speech ... and back)  
//    read_and_parse("hobbit.txt", our_text); // 2 types of citation marks
//    read_and_parse2("hobbit_mod.txt", our_text); // 1 type of citation marks
//    read_and_parse2("three_men_in_a_boat.txt", our_text); // ???
//    read_and_parse2("win1.txt", our_text); // 1 type of citation marks

    
//    color_scintilla();


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

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
  WNDCLASSEXW wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ED));
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_ED);
  wcex.lpszClassName = szWindowClass;
  wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

  return RegisterClassExW(&wcex);
}

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
    case WM_CREATE:
      hwndScintilla = CreateWindowW(L"Scintilla", L"r-e-a-d (C) 2020 EkwoTECH GmbH, Friedrichshafen",
        WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, hWnd, nullptr, hInst, nullptr);
      ShowWindow(hwndScintilla, SW_SHOW);
      UpdateWindow(hwndScintilla);
      break;
    case WM_SIZE:
      if (wParam != 1) {
        RECT rc;
        ::GetClientRect(hWnd, &rc);
        ::SetWindowPos(hwndScintilla, 0, 100/*rc.left*/, rc.top, rc.right - rc.left-100, rc.bottom - rc.top, 0);
      }
      return 0;
    
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_LOAD:
              // open a file name, https://www.daniweb.com/programming/software-development/code/217307/a-simple-getopenfilename-example
              ZeroMemory(&ofn, sizeof(ofn));
              ofn.lStructSize = sizeof(ofn);
              ofn.hwndOwner = hWnd;
              ofn.lpstrFile = (WCHAR*)szFile;
//              ofn.lpstrFile = '\0';
              ofn.nMaxFile = sizeof(szFile);
              ofn.lpstrFilter = L"Text\0 * .TXT\0All\0*.*";
              ofn.nFilterIndex = 1;
              ofn.lpstrFileTitle = NULL;
              ofn.nMaxFileTitle = 0;
              ofn.lpstrInitialDir = NULL;
              ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
              b_filechosen = GetOpenFileName(&ofn);
              if (b_filechosen)
              {
//                MessageBox(NULL, ofn.lpstrFile, L"File Name", MB_OK);
                char buffer[500];
                size_t charsConverted = 0;
                // First arg is the pointer to destination char, second arg is
                // the pointer to source wchar_t, last arg is the size of char buffer
                wcstombs_s(&charsConverted, buffer, ofn.lpstrFile, 500);
                std::string fname(buffer);
                read_and_parse2(fname, our_text); // 1 type of citation marks
                color_scintilla();
              }
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


void color_scintilla()
{
  // (A) fast version [...]
// (B) regular (slow?) version
  SendMessage(hwndScintilla, SCI_SETTEXT, 0, int(our_text.text.c_str()));

  // (i) define styles
  // style 0
  SendMessage(hwndScintilla, SCI_STYLESETFORE, 0, 0x0040FF);
  // style 1
  SendMessage(hwndScintilla, SCI_STYLESETFORE, 1, 0x804000); // dark blue (bgr!)
  SendMessage(hwndScintilla, SCI_STYLESETBACK, 1, 0x00FF80); // light green (bgr!)
  // style 2 (Bilbo)
  SendMessage(hwndScintilla, SCI_STYLESETFORE, 2, 0x804000); // dark blue (bgr!)
  SendMessage(hwndScintilla, SCI_STYLESETBACK, 2, 0xE9C2CE); // light violet (bgr!)


  // (ii) now just concatenate colored/styled sections
  SendMessage(hwndScintilla, SCI_STARTSTYLING, 0, 1); // SCI_STARTSTYLING(position start, int unused)
  // do the coloring
  std::list<speech_at>::iterator it;
  int cnt = 0;
  int pos_prev = 0;
  for (it = our_text.list_of_speech.begin(); it != our_text.list_of_speech.end(); ++it)
  {
    int pos = it->pos;
    int style = it->type;
    SendMessage(hwndScintilla, SCI_SETSTYLING, pos - pos_prev, style);
    pos_prev = pos;
  }
}