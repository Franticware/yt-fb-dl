#define UNICODE
#include <windows.h>

#include <cstdio>

#include "zip.h"

#define ID_EDIT_URL 1
#define ID_BUTTON_DL 2
#define ID_BUTTON_UPDATE 3
#define IDT_TIMER1 4
#define IDI_APP 100

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static HWND hwndEdit;
static HWND hwndEditOut;
static HWND hwndButton;
static HWND hwndButtonUpd;
static HWND hwndStaticUpd;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;
    WNDCLASSW wc = {0};
    wc.lpszClassName = TEXT("yt-fb-dl");
    wc.hInstance = hInstance;
    wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
    wc.lpfnWndProc = WndProc;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_APP));

    RegisterClass(&wc);
    CreateWindow(
      wc.lpszClassName, TEXT("https://github.com/Franticware/yt-fb-dl"), WS_OVERLAPPEDWINDOW | WS_VISIBLE, 220, 220, 870, 200, 0, 0, hInstance, 0);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CREATE:
            hwndEdit = CreateWindow(TEXT("Edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 90, 43, 750, 26, hwnd, (HMENU)ID_EDIT_URL, NULL, NULL);
            hwndButton = CreateWindow(TEXT("button"), TEXT("Download"), WS_VISIBLE | WS_CHILD, 5, 40, 80, 30, hwnd, (HMENU)ID_BUTTON_DL, NULL, NULL);
            hwndStaticUpd = CreateWindow(TEXT("Static"), TEXT(""), WS_CHILD | WS_VISIBLE | SS_LEFT, 90, 8, 750, 26, hwnd, (HMENU)1, NULL, NULL);
            hwndButtonUpd =
              CreateWindow(TEXT("button"), TEXT("Update"), WS_VISIBLE | WS_CHILD, 5, 5, 80, 30, hwnd, (HMENU)ID_BUTTON_UPDATE, NULL, NULL);
            break;

        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                if (LOWORD(wParam) == ID_BUTTON_UPDATE)
                {
                    EnableWindow(hwndButton, false);
                    EnableWindow(hwndButtonUpd, false);

                    SetWindowText(hwndStaticUpd, TEXT("Updating yt-dlp..."));

                    HRESULT res = URLDownloadToFile(
                      NULL, TEXT("https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp_x86.exe"), TEXT("yt-dlp_x86.exe"), 0, NULL);
                    if (res == S_OK)
                    {
                        SetWindowText(hwndStaticUpd, TEXT("Updating yt-dlp... done. Updating ffmpeg..."));
                        res = URLDownloadToFile(
                          NULL,
                          TEXT("https://github.com/sudo-nautilus/FFmpeg-Builds-Win32/releases/download/latest/ffmpeg-master-latest-win32-gpl.zip"),
                          TEXT("ffmpeg.zip"),
                          0,
                          NULL);

                        if (res == S_OK)
                        {
                            struct zip_t* zip = zip_open("ffmpeg.zip", 0, 'r');
                            {
                                zip_entry_open(zip, "ffmpeg-master-latest-win32-gpl/bin/ffmpeg.exe");
                                {
                                    zip_entry_fread(zip, "ffmpeg.exe");
                                }
                                zip_entry_close(zip);
                            }
                            zip_close(zip);

                            SetWindowText(hwndStaticUpd, TEXT("Updating yt-dlp... done. Updating ffmpeg... done."));
                        }
                        else
                        {
                            SetWindowText(hwndStaticUpd, TEXT("Updating yt-dlp... done. Updating ffmpeg... ERROR!"));
                        }
                    }
                    else
                    {
                        SetWindowText(hwndStaticUpd, TEXT("Updating yt-dlp... ERROR!"));
                    }

                    EnableWindow(hwndButton, true);
                    EnableWindow(hwndButtonUpd, true);
                }
                else if (LOWORD(wParam) == ID_BUTTON_DL)
                {
                    int len = GetWindowTextLength(hwndEdit) + 1;
                    wchar_t text[len];
                    GetWindowText(hwndEdit, text, len);
                    char text8[1024] = {0};

                    WideCharToMultiByte(CP_UTF8, 0, text, -1, text8, 1023, NULL, NULL);

                    FILE* batFile = fopen("proc.bat", "wb");
                    fputs("chcp 65001\r\n", batFile);
                    fputs("yt-dlp_x86.exe --no-colors --restrict-filenames --windows-filenames --trim-filenames 32 ", batFile);
                    fputs(text8, batFile);
                    fputs("\r\n", batFile);
                    fputs("pause\r\n", batFile);
                    fclose(batFile);

                    TCHAR szCmdline[] = TEXT("proc.bat");

                    EnableWindow(hwndButton, false);
                    EnableWindow(hwndButtonUpd, false);

                    STARTUPINFO info = {sizeof(info)};
                    PROCESS_INFORMATION processInfo;
                    if (CreateProcess(NULL, szCmdline, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &info, &processInfo))
                    {
                        WaitForSingleObject(processInfo.hProcess, INFINITE);
                        CloseHandle(processInfo.hProcess);
                        CloseHandle(processInfo.hThread);
                    }

                    EnableWindow(hwndButton, true);
                    EnableWindow(hwndButtonUpd, true);
                }
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
