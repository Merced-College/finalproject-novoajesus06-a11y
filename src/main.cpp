/*
 * Author: [Your Name]
 * File:   main.cpp
 *
 * Description:
 * Win32 entry point for Platform Quest.
 * Creates the window, sets up a fixed-timestep game loop using
 * SetTimer, and routes WM_KEYDOWN/WM_KEYUP to the Game object.
 *
 * To compile in Visual Studio:
 *   - Create a new "Windows Desktop Application" project (NOT Console)
 *   - Add all .h and .cpp files from the src/ folder
 *   - Build & Run (F5)
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <string>
#include "Game.h"

// ── Globals ───────────────────────────────────────────────────────────────────
static Game*      g_game    = nullptr;
static HWND       g_hwnd    = nullptr;
static LARGE_INTEGER g_freq = {};
static LARGE_INTEGER g_prev = {};

static const int TIMER_ID  = 1;
static const int TARGET_FPS = 60;

// ── Simple name input dialog ──────────────────────────────────────────────────
static std::string InputBox(HWND parent, const char* prompt, const char* title) {
    // We implement a minimal dialog using a standard MessageBox-style approach.
    // For a real project you'd define a dialog resource; here we use a quick
    // common dialog workaround via a hidden edit control on a message box.
    // Simplest portable approach: just return a default and let the user rename.
    // Visual Studio supports InputBox via a modal dialog.

    struct DlgData { const char* prompt; char buf[64]; };
    static DlgData data;
    data.prompt = prompt;
    data.buf[0] = '\0';

    // Use a dialog proc via DialogBoxIndirect for a real in-app input box
    // without needing resource files.
    struct Local {
        static INT_PTR CALLBACK Proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
            switch (msg) {
            case WM_INITDIALOG: {
                HWND hLabel = GetDlgItem(hDlg, 101);
                HWND hEdit  = GetDlgItem(hDlg, 102);
                if (hLabel) SetWindowTextA(hLabel, ((DlgData*)lParam)->prompt);
                if (hEdit)  SetFocus(hEdit);
                return TRUE;
            }
            case WM_COMMAND:
                if (LOWORD(wParam) == IDOK) {
                    HWND hEdit = GetDlgItem(hDlg, 102);
                    if (hEdit) GetWindowTextA(hEdit, data.buf, sizeof(data.buf));
                    EndDialog(hDlg, IDOK);
                }
                if (LOWORD(wParam) == IDCANCEL) EndDialog(hDlg, IDCANCEL);
                return TRUE;
            }
            return FALSE;
        }
    };

    // Build dialog template in memory
    struct { DLGTEMPLATE tmpl; WORD menu, cls, title[16];
             DLGITEMTEMPLATE label; WORD lcls[2]; WORD ltext[32];
             DLGITEMTEMPLATE edit;  WORD ecls[2]; WORD etext;
             DLGITEMTEMPLATE btn;   WORD bcls[2]; WORD btext[3]; } dlg = {};

    dlg.tmpl.style          = WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME | DS_CENTER;
    dlg.tmpl.cdit           = 3;
    dlg.tmpl.cx             = 200; dlg.tmpl.cy = 80;
    // title text (wide)
    const wchar_t wtitle[]  = L"Enter Name";
    for (int i = 0; i < 10; ++i) dlg.title[i] = wtitle[i];

    // Label
    dlg.label.style         = WS_CHILD | WS_VISIBLE | SS_LEFT;
    dlg.label.x = 7;  dlg.label.y = 10; dlg.label.cx = 186; dlg.label.cy = 14;
    dlg.label.id            = 101;
    dlg.lcls[0]             = 0xFFFF; dlg.lcls[1] = 0x0082; // STATIC
    // Edit
    dlg.edit.style          = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL;
    dlg.edit.x = 7;  dlg.edit.y = 28; dlg.edit.cx = 186; dlg.edit.cy = 14;
    dlg.edit.id             = 102;
    dlg.ecls[0]             = 0xFFFF; dlg.ecls[1] = 0x0081; // EDIT
    // OK button
    dlg.btn.style           = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON;
    dlg.btn.x = 75; dlg.btn.y = 52; dlg.btn.cx = 50; dlg.btn.cy = 14;
    dlg.btn.id              = IDOK;
    dlg.bcls[0]             = 0xFFFF; dlg.bcls[1] = 0x0080; // BUTTON
    dlg.btext[0]            = 'O'; dlg.btext[1] = 'K'; dlg.btext[2] = 0;

    INT_PTR ret = DialogBoxIndirectParamA(
        GetModuleHandle(nullptr),
        (LPDLGTEMPLATE)&dlg,
        parent,
        Local::Proc,
        (LPARAM)&data);

    return (ret == IDOK && data.buf[0]) ? std::string(data.buf) : "Player";
}

// ── Window procedure ──────────────────────────────────────────────────────────
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_TIMER: {
        if (!g_game) break;

        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        float dt = (float)(now.QuadPart - g_prev.QuadPart) / g_freq.QuadPart;
        g_prev = now;
        if (dt > 0.05f) dt = 0.05f;

        g_game->update(dt);
        InvalidateRect(hwnd, nullptr, FALSE);

        if (!g_game->isRunning()) PostQuitMessage(0);
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if (g_game) {
            RECT rc; GetClientRect(hwnd, &rc);
            g_game->render(hdc, rc.right, rc.bottom);
        }
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_ACTIVATE:
        // Re-grab focus whenever window is clicked or activated
        SetFocus(hwnd);
        break;

    case WM_LBUTTONDOWN:
        SetFocus(hwnd);
        break;

    case WM_KEYDOWN:
        if (g_game) g_game->keyDown((int)wParam);
        break;

    case WM_KEYUP:
        if (g_game) g_game->keyUp((int)wParam);
        break;

    case WM_DESTROY:
        KillTimer(hwnd, TIMER_ID);
        PostQuitMessage(0);
        break;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

// ── Entry point ───────────────────────────────────────────────────────────────
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    // Register window class
    WNDCLASSA wc      = {};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.lpszClassName = "PlatformQuestClass";
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.hIcon         = LoadIcon(nullptr, IDI_APPLICATION);
    RegisterClassA(&wc);

    // Create window
    g_hwnd = CreateWindowA(
        "PlatformQuestClass", "Platform Quest",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1100, 650,
        nullptr, nullptr, hInst, nullptr);

    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);
    // Force window to front and grab keyboard focus
    SetForegroundWindow(g_hwnd);
    SetFocus(g_hwnd);
    BringWindowToTop(g_hwnd);

    // Create game — player types name on the menu screen
    Game game;
    game.setPlayerName("Player");
    g_game = &game;

    // Start game loop timer (~60 FPS)
    QueryPerformanceFrequency(&g_freq);
    QueryPerformanceCounter(&g_prev);
    SetTimer(g_hwnd, TIMER_ID, 1000 / TARGET_FPS, nullptr);

    // Message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
