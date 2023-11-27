#define UNICODE
#include <windows.h>

#include <shlobj.h>

#include "util.h"
#include "zip.h"
#include <cstdio>

#define ID_EDIT_URL 1
#define ID_BUTTON_DOWNLOAD 2
#define ID_BUTTON_UPDATE 3
#define IDT_TIMER1 4
#define ID_CHECKBOX_FLAG 64
#define ID_CHECKBOX_FFMPEG (5 | ID_CHECKBOX_FLAG)
#define ID_CHECKBOX_MP4 (6 | ID_CHECKBOX_FLAG)

#define IDI_APP 100

#define STR_ORG "Franticware"
#define STR_APP "yt-fb-dl"
#define STR_FILE_YT_DLP_EXE "yt-dlp_x86.exe"
#define STR_FILE_FFMPEG_EXE "ffmpeg.exe"
#define STR_FILE_FFMPEG_ZIP "ffmpeg.zip"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static HWND hwndEditUrl;
static HWND hwndButtonDownload;
static HWND hwndButtonUpdate;
static HWND hwndStaticUpdate;
static HWND hwndCheckboxFfmpeg;
static HWND hwndCheckboxMp4;

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

static void Update(HWND hwnd)
{
    EnableWindow(hwndButtonDownload, false);
    EnableWindow(hwndButtonUpdate, false);

    SetWindowText(hwndStaticUpdate, TEXT("Updating yt-dlp..."));

    TCHAR ytdlpExePath[MAX_PATH] = {0};
    GetAppDataPath(ytdlpExePath, TEXT(STR_ORG), TEXT(STR_APP), TEXT(STR_FILE_YT_DLP_EXE));

    HRESULT res = URLDownloadToFile(NULL, TEXT("https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp_x86.exe"), ytdlpExePath, 0, NULL);
    if (res == S_OK)
    {
        SetWindowText(hwndStaticUpdate, TEXT("Updating yt-dlp... done."));

        TCHAR ffmpegExePath[MAX_PATH] = {0};
        GetAppDataPath(ffmpegExePath, TEXT(STR_ORG), TEXT(STR_APP), TEXT(STR_FILE_FFMPEG_EXE));

        if (!IsDlgButtonChecked(hwnd, ID_CHECKBOX_FFMPEG) || !FileExists(ffmpegExePath))
        {
            SetWindowText(hwndStaticUpdate, TEXT("Updating yt-dlp... done. Updating ffmpeg..."));
            TCHAR ffmpegZipPath[MAX_PATH] = {0};
            GetAppDataPath(ffmpegZipPath, TEXT(STR_ORG), TEXT(STR_APP), TEXT(STR_FILE_FFMPEG_ZIP));

            char ffmpegZipPath8[MAX_PATH8] = {0};
            pathToUtf8(ffmpegZipPath8, ffmpegZipPath);

            res = URLDownloadToFile(
              NULL,
              TEXT("https://github.com/sudo-nautilus/FFmpeg-Builds-Win32/releases/download/latest/ffmpeg-master-latest-win32-gpl.zip"),
              ffmpegZipPath,
              0,
              NULL);

            if (res == S_OK)
            {
                struct zip_t* zip = zip_open(ffmpegZipPath8, 0, 'r');
                {
                    zip_entry_open(zip, "ffmpeg-master-latest-win32-gpl/bin/ffmpeg.exe");
                    {
                        char ffmpegExePath8[MAX_PATH8] = {0};
                        pathToUtf8(ffmpegExePath8, ffmpegExePath);

                        zip_entry_fread(zip, ffmpegExePath8);
                    }
                    zip_entry_close(zip);
                }
                zip_close(zip);

                SetWindowText(hwndStaticUpdate, TEXT("Updating yt-dlp... done. Updating ffmpeg... done."));
            }
            else
            {
                SetWindowText(hwndStaticUpdate, TEXT("Updating yt-dlp... done. Updating ffmpeg... ERROR!"));
            }
        }
    }
    else
    {
        SetWindowText(hwndStaticUpdate, TEXT("Updating yt-dlp... ERROR!"));
    }

    EnableWindow(hwndButtonDownload, true);
    EnableWindow(hwndButtonUpdate, true);
}

static void Download(HWND hwnd)
{
    int len = GetWindowTextLength(hwndEditUrl);
    if (len <= 0)
    {
        MessageBox(hwnd, TEXT("Please enter the download URL."), TEXT("Empty download URL"), MB_OK | MB_ICONEXCLAMATION);
        SetFocus(hwndEditUrl);
    }
    else
    {
        wchar_t url[len + 1] = {0};
        GetWindowText(hwndEditUrl, url, len + 1);
        char url8[MAX_PATH8] = {0};
        pathToUtf8(url8, url);

        TCHAR startingFolder[MAX_PATH] = {0};
        if (!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_MYVIDEO | CSIDL_FLAG_CREATE, NULL, 0, startingFolder)))
        {
            startingFolder[0] = 0;
        }
        TCHAR outputFolder[MAX_PATH] = {0};
        if (BrowseForFolder(hwnd, outputFolder, TEXT("Select download folder"), startingFolder))
        {
            {
                TCHAR ytdlpExePath[MAX_PATH] = {0};
                GetAppDataPath(ytdlpExePath, TEXT(STR_ORG), TEXT(STR_APP), TEXT(STR_FILE_YT_DLP_EXE));
                TCHAR ffmpegExePath[MAX_PATH] = {0};
                GetAppDataPath(ffmpegExePath, TEXT(STR_ORG), TEXT(STR_APP), TEXT(STR_FILE_FFMPEG_EXE));
                if (!FileExists(ytdlpExePath) || !FileExists(ffmpegExePath))
                {
                    Update(hwnd);
                }
            }

            TCHAR batName[MAX_PATH] = {0};
            DWORD ticks = GetTickCount();
            _snwprintf(batName, MAX_PATH - 1, L"proc%u.bat", ticks);

            TCHAR batPath[MAX_PATH] = {0};
            GetAppDataPath(batPath, TEXT(STR_ORG), TEXT(STR_APP), batName);

            char ytdlpExePath8[MAX_PATH8] = {0};
            GetAppDataPath(ytdlpExePath8, TEXT(STR_ORG), TEXT(STR_APP), TEXT(STR_FILE_YT_DLP_EXE));

            {
                FILE* batFile = _wfopen(batPath, L"wb");
                fputs("chcp 65001\r\n", batFile);
                fputs("\"", batFile);
                fputs(ytdlpExePath8, batFile);
                fputs("\" -U --no-colors --restrict-filenames --windows-filenames --trim-filenames 32 ", batFile);
                if (IsDlgButtonChecked(hwnd, ID_CHECKBOX_MP4))
                {
                    fputs("-f \"bestvideo[ext=mp4]+bestaudio[ext=m4a]/best[ext=mp4]/best\" ", batFile);
                }
                fputs(url8, batFile);
                fputs("\r\n", batFile);
                fputs("pause\r\n", batFile);
                fclose(batFile);
            }

            EnableWindow(hwndButtonDownload, false);
            EnableWindow(hwndButtonUpdate, false);

            STARTUPINFO info = {sizeof(info)};
            PROCESS_INFORMATION processInfo;
            if (CreateProcess(NULL, batPath, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, outputFolder, &info, &processInfo))
            {
                WaitForSingleObject(processInfo.hProcess, INFINITE);
                CloseHandle(processInfo.hProcess);
                CloseHandle(processInfo.hThread);
            }

            DeleteFile(batPath);

            EnableWindow(hwndButtonDownload, true);
            EnableWindow(hwndButtonUpdate, true);
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CREATE:
            hwndButtonUpdate =
              CreateWindow(TEXT("button"), TEXT("Update"), WS_VISIBLE | WS_CHILD, 5, 5, 80, 30, hwnd, (HMENU)ID_BUTTON_UPDATE, NULL, NULL);
            hwndStaticUpdate = CreateWindow(TEXT("Static"), TEXT(""), WS_CHILD | WS_VISIBLE | SS_LEFT, 90, 8, 750, 26, hwnd, (HMENU)1, NULL, NULL);
            hwndButtonDownload =
              CreateWindow(TEXT("button"), TEXT("Download"), WS_VISIBLE | WS_CHILD, 5, 40, 80, 30, hwnd, (HMENU)ID_BUTTON_DOWNLOAD, NULL, NULL);
            hwndEditUrl = CreateWindow(TEXT("Edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 90, 43, 750, 26, hwnd, (HMENU)ID_EDIT_URL, NULL, NULL);
            hwndCheckboxFfmpeg = CreateWindow(TEXT("button"),
                                              TEXT("Do not update existing ffmpeg"),
                                              WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
                                              5,
                                              90,
                                              750,
                                              26,
                                              hwnd,
                                              (HMENU)ID_CHECKBOX_FFMPEG,
                                              NULL,
                                              NULL);
            CheckDlgButton(hwnd, ID_CHECKBOX_FFMPEG, BST_CHECKED);
            hwndCheckboxMp4 = CreateWindow(
              TEXT("button"), TEXT("Prefer mp4"), WS_VISIBLE | WS_CHILD | BS_CHECKBOX, 5, 116, 750, 26, hwnd, (HMENU)ID_CHECKBOX_MP4, NULL, NULL);
            CheckDlgButton(hwnd, ID_CHECKBOX_MP4, BST_UNCHECKED);
            break;

        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                const WORD id = LOWORD(wParam);
                if (id == ID_BUTTON_UPDATE)
                {
                    Update(hwnd);
                }
                else if (id == ID_BUTTON_DOWNLOAD)
                {
                    Download(hwnd);
                }

                if (id & ID_CHECKBOX_FLAG)
                {
                    BOOL checked = IsDlgButtonChecked(hwnd, id);
                    if (checked)
                    {
                        CheckDlgButton(hwnd, id, BST_UNCHECKED);
                    }
                    else
                    {
                        CheckDlgButton(hwnd, id, BST_CHECKED);
                    }
                }
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
