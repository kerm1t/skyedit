// ed.cpp : Defines the entry point for the application.
// this should probably renamed to read.cpp or main.cpp

#include "stdafx.h"
#include "ed.h"
#include "Scintilla.h" // 2do: move into this repo
#include "read_and_parse.h"
#include <Commdlg.h>
//#include <windows.h> // CFont, nope


highlighted_corpus our_text;
std::vector<speaker> speakers;

HWND hList;
void listbox_add_speakers();

HWND hwndScintilla;
OPENFILENAME ofn;
char szFile[260];
BOOL b_filechosen;
void scintilla_fill();
void scintilla_init_styles();
void scintilla_color();


//define an unicode string type alias
typedef std::basic_string<TCHAR> ustring;
//=============================================================================
//message processing function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int OnCreate(const HWND, CREATESTRUCT*);

//non-message function declarations
inline UINT AddString(const HWND, const std::wstring&);
HWND CreateListbox(const HWND, const HINSTANCE, DWORD, const RECT&, const int, const ustring&);
HWND CreateStatics(const HWND, const HINSTANCE, DWORD, const RECT&, const int, const ustring&); 
inline int ErrMsg(const ustring&);

//setup some edit control id's
enum {
  IDCL_LISTBOX = 200,
  IDC_TEXT
};
//=============================================================================

// helpers ...
int StringToWString(std::wstring &ws, const std::string &s)
{
  std::wstring wsTmp(s.begin(), s.end());
  ws = wsTmp;
  return 0;
}



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

    // show main (parent) window
    // 2do: add more controls, i.e. a listbox
    HWND hwndBase = CreateWindowW(szWindowClass, L"r-e-a-d (C) 2020 EkwoTECH GmbH, Friedrichshafen", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
    if (!hwndBase)
    {
      return FALSE;
    }
    ShowWindow(hwndBase, nCmdShow);
    UpdateWindow(hwndBase);

    SendMessage(hwndScintilla, SCI_SETMARGINWIDTHN, 0, 60); //  show line numbers

    scintilla_init_styles(); // do this only once


    // show a dummy text, which gives some debugging help, too
    std::string dummy_txt = "\"Peter und der Wolf.\"\nvon Sergei Prokofiev (1891-1953)\n\nOpen a new file\nto get speech and speaker recognition.";
    SendMessage(hwndScintilla, SCI_SETTEXT, 0, reinterpret_cast<LPARAM>(dummy_txt.c_str()));
///    SendMessage(hwndScintilla, SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM)"Calibri"); // font
///    SendMessage(hwndScintilla, SCI_STYLESETSIZE, STYLE_DEFAULT, 48); // font size
    our_text.tag_list.push_back({ 1, (text_type)1, MIN_STYLE_SPEAKER+1 });  // 0 up to here = light green
    our_text.tag_list.push_back({ 6, (text_type)0, MIN_STYLE_SPEAKER+0 });  // up to here
    our_text.tag_list.push_back({ 21, (text_type)2, MIN_STYLE_SPEAKER+2 }); // up to here
    scintilla_color();
//    SendMessage(hwndScintilla, SCI_ANNOTATIONSETTEXT, 1, reinterpret_cast<LPARAM>(L"watt is datt denn")); // in line 1
    std::string s = "Test, dies ist eine Bemerkung in Zeile 2";
    SendMessage(hwndScintilla, SCI_ANNOTATIONSETTEXT, 1, reinterpret_cast<LPARAM>(s.c_str())); // in line 1
    SendMessage(hwndScintilla, SCI_ANNOTATIONSETSTYLE, 1, 3); // style 3
    SendMessage(hwndScintilla, SCI_ANNOTATIONSETVISIBLE, 2, 0); // ANNOTATION_BOXED	2	Annotations are indented to match the text and are surrounded by a box.


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
      hwndScintilla = CreateWindowW(L"scintilla", L"r-e-a-d (C) 2020 EkwoTECH GmbH, Friedrichshafen",
        WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, hWnd, nullptr, hInst, nullptr);
      ShowWindow(hwndScintilla, SW_SHOW);
      UpdateWindow(hwndScintilla);
      
      OnCreate(hWnd, reinterpret_cast<CREATESTRUCT*>(lParam));
      
      break;
    case WM_SIZE:
      if (wParam != 1) {
        RECT rc;
        ::GetClientRect(hWnd, &rc);
        ::SetWindowPos(hwndScintilla, 0, 150/*rc.left*/, rc.top, rc.right - rc.left-150, rc.bottom - rc.top, 0);
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
                char buffer[500]; // max. size of dir + filename!
                size_t charsConverted = 0;
                // First arg is the pointer to destination char, second arg is
                // the pointer to source wchar_t, last arg is the size of char buffer
// https://stackoverflow.com/questions/18645874/converting-stdwsting-to-char-with-wcstombs-s
                wcstombs_s(&charsConverted, buffer, ofn.lpstrFile, 500);
                std::string fname(buffer);
                read_and_parse2(fname, our_text, speakers); // 1 type of citation marks
                listbox_add_speakers();
                scintilla_fill();
                scintilla_color();
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

void listbox_add_speakers()
{
  sort(speakers.begin(), speakers.end());
  SendMessage(hList, LB_RESETCONTENT, 0, 0); // clear
  for (int i = 0; i < (int)speakers.size(); i++) // 2do: use iterator to traverse through std::vector
  {
    std::string s = speakers[i].name + " (" + std::to_string(speakers[i].occurence) + ")";
    std::wstring ws;
    StringToWString(ws, s);
    AddString(hList, ws);
  }
}

void scintilla_fill()
{
  // (A) fast version [...]
// (B) regular (slow?) version
  SendMessage(hwndScintilla, SCI_SETTEXT, 0, reinterpret_cast<LPARAM>(our_text.text.c_str()));
  //  std::string tmp_speakers(*speaker.data());
  //  SendMessage(hwndScintilla, SCI_SETTEXT, 0, int(tmp_speakers.c_str()));
}

void scintilla_init_styles()
{
  // https://scintilla-interest.narkive.com/j9CxOCCw/changing-the-font-doesn-t-update-scintilla
  SendMessage(hwndScintilla, SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM)"Calibri"); // font
  SendMessage(hwndScintilla, SCI_STYLESETSIZE, STYLE_DEFAULT, 20); // font size
  SendMessage(hwndScintilla, SCI_STYLECLEARALL, 0, 0);

  SendMessage(hwndScintilla, SCI_SETWRAPMODE, 1, 1);

  // (i) define styles
  // style 0
  SendMessage(hwndScintilla, SCI_STYLESETFORE, 0, 0x000000);
  // style 1 (new speaker recognized)
  SendMessage(hwndScintilla, SCI_STYLESETFORE, 1, 0x000000); // black
  SendMessage(hwndScintilla, SCI_STYLESETBOLD, 1, 1);        // bold  
  // style 2 (known speaker recognized!)
  SendMessage(hwndScintilla, SCI_STYLESETFORE, 2, 0x0000FF); // red (bgr!)
  SendMessage(hwndScintilla, SCI_STYLESETUNDERLINE, 2, 1);   // underline
  SendMessage(hwndScintilla, SCI_STYLESETBOLD, 2, 1);        // bold
  // style 3 (citation found, speaker undefined)
  SendMessage(hwndScintilla, SCI_STYLESETFORE, 3, 0x804000); // dark blue (bgr!)
  SendMessage(hwndScintilla, SCI_STYLESETBACK, 3, 0xCCCCCC); // light gray (bgr!)
//  tut's net mit diesem style, nur fuer STYLE_DEFAULT ... (s.o.)
//  SendMessage(hwndScintilla, SCI_STYLESETFONT, 1, (LPARAM)"Arial"); // font
//  SendMessage(hwndScintilla, SCI_STYLESETSIZE, 1, 20); // font size
//  SendMessage(hwndScintilla, SCI_STYLECLEARALL,1, 0);
  // style 4 (keyword "said", "answered", ... recognized)
  SendMessage(hwndScintilla, SCI_STYLESETFORE, 4, 0x804000); // dark blue (bgr!)
  SendMessage(hwndScintilla, SCI_STYLESETBACK, 4, 0xE9C2CE); // light violet (bgr!)
  // style 5
  SendMessage(hwndScintilla, SCI_STYLESETFORE, 5, 0x804000); // dark blue (bgr!)
  SendMessage(hwndScintilla, SCI_STYLESETBACK, 5, 0x00FF80); // light green (bgr!)
  std::srand(std::time(nullptr)); // seed
#define HAND_DEFINED_STYLES 6
  // 2do: dark background -> white font ( wenn 2 von 3 kleiner 0x1d)
  for (int i = HAND_DEFINED_STYLES; i < 30; i++)
  {
    // (a) background color
#define COLOR_MIN 35
    int tmp1 = COLOR_MIN + (std::rand() % (252 - COLOR_MIN)); // limit to 252, otherwise we might get white background
    int tmp2 = COLOR_MIN + (std::rand() % (252 - COLOR_MIN));
    int tmp3 = COLOR_MIN + (std::rand() % (252 - COLOR_MIN));
    int bgcol = (tmp1 << 16) | (tmp2 << 8) | tmp3;
    SendMessage(hwndScintilla, SCI_STYLESETBACK, i, bgcol); // light green (bgr!)
    // (b) foreground color, this is quite a crude heuristic, 2do: eventually replace by some better color model
    int contrast1 = 0;
    int contrast2 = 0;
#define LOW_CONTRAST1 100
#define LOW_CONTRAST2 30 // 50 // 30
    if (tmp1 < LOW_CONTRAST1) contrast1++;
    if (tmp2 < LOW_CONTRAST1) contrast1++;
    if (tmp3 < LOW_CONTRAST1) contrast1++;
    if (tmp1 < LOW_CONTRAST2) contrast2++;
    if (tmp2 < LOW_CONTRAST2) contrast2++;
    if (tmp3 < LOW_CONTRAST2) contrast2++;
    if ((contrast1 >= 1) ||
      (contrast2 >= 2))
      SendMessage(hwndScintilla, SCI_STYLESETFORE, i, 0xFFFFFF); // white
    else
      SendMessage(hwndScintilla, SCI_STYLESETFORE, i, 0x804000); // dark blue (bgr!)
  }
}

void scintilla_color()
{
  // (ii) now just concatenate colored/styled sections
///  SendMessage(hwndScintilla, SCI_STARTSTYLING, 2, 1); // only needed, if style start is > 0
  // do the coloring
  std::list<text_tag>::iterator it;
  //  int cnt = 0;
  int pos_prev = 0;
  for (it = our_text.tag_list.begin(); it != our_text.tag_list.end(); ++it)
  {
    int pos = it->pos;
///    int style = it->type;
    int idx_speaker = it->idx_speaker;
//    if (cnt==0)
//      SendMessage(hwndScintilla, SCI_SETSTYLING, pos - pos_prev, 0);
//    else
///      SendMessage(hwndScintilla, SCI_SETSTYLING, pos - pos_prev, style);
      SendMessage(hwndScintilla, SCI_SETSTYLING, pos - pos_prev, idx_speaker);
      pos_prev = pos;
//    cnt++;
  }
}

// http://winapi.foosyerdoos.org.uk/code/usercntrls/htm/createlistbox.php
//=============================================================================
inline int ErrMsg(const ustring& s)
{
  return MessageBox(0, s.c_str(), _T("ERROR"), MB_OK | MB_ICONEXCLAMATION);
}
//=============================================================================
int OnCreate(const HWND hwnd, CREATESTRUCT *cs)
{
  //handles the WM_CREATE message of the main, parent window; return -1 to fail
  //window creation
  RECT rect;
  int height;
  if (GetClientRect(hwnd, &rect)) { height = rect.bottom - rect.top; }
  RECT rc;
  rc = { 0,0,150,20 };
  CreateStatics(hwnd, cs->hInstance, SS_SIMPLE, rc, IDC_TEXT, _T("Speakers"));
  rc = { 0,20,150, height };
  hList = CreateListbox(hwnd, cs->hInstance, 0, rc, IDCL_LISTBOX, _T(""));
  
//  CFont Font;
//  Font.CreatePointFont(120, _T("Courier"));        // creates a 12-point-Courier-font
//  hList.SetFont(&Font);                        // with a member variable associated
///  GetDlgItem(IDCL_LISTBOX)->SetFont(&Font);         // without a member variable associated
//  AddString(hList, _T("Listbox"));

  return 0;
}
//=============================================================================
HWND CreateListbox(const HWND hParent, const HINSTANCE hInst, DWORD dwStyle,
  const RECT& rc, const int id, const ustring& caption)
{
  dwStyle |= WS_CHILD | WS_VISIBLE | WS_VSCROLL;
  return CreateWindowEx(WS_EX_CLIENTEDGE,             //extended styles
    _T("listbox"),                //control 'class' name
    caption.c_str(),              //control caption
    dwStyle,                      //control style 
    rc.left,                      //position: left
    rc.top,                       //position: top
    rc.right,                     //width
    rc.bottom,                    //height
    hParent,                      //parent window handle
    //control's ID
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
    hInst,                        //application instance
    0);                           //user defined info
}
//=============================================================================
HWND CreateStatics(const HWND hParent, const HINSTANCE hInst, DWORD dwStyle,
  const RECT& rc, const int id, const ustring& caption)
{
  dwStyle |= WS_CHILD | WS_VISIBLE;
  return CreateWindowEx(0,                            //extended styles
    _T("static"),                 //control 'class' name
    caption.c_str(),              //control caption
    dwStyle,                      //control style 
    rc.left,                      //position: left
    rc.top,                       //position: top
    rc.right,                     //width
    rc.bottom,                    //height
    hParent,                      //parent window handle
    //control's ID
    reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
    hInst,                        //application instance
    0);                           //user defined info
}
//=============================================================================
inline UINT AddString(const HWND hList, const std::wstring& s)
{
  return static_cast<UINT>(SendMessage(hList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s.c_str())));
}
//=============================================================================