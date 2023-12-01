#ifndef UTIL_H
#define UTIL_H

#define UNICODE
#include <windows.h>

#define MAX_PATH8 1024

inline void pathToUtf8(char* path8, LPCTSTR pathw)
{
    path8[MAX_PATH8 - 1] = 0;
    WideCharToMultiByte(CP_UTF8, 0, pathw, -1, path8, MAX_PATH8 - 1, NULL, NULL);
}

bool unzipFile(LPCTSTR zipPath, LPCTSTR zippedPath, LPCTSTR outputPath);

bool FileExists(LPCTSTR szPath);
void GetAppDataPath(LPTSTR resultingPath, LPCTSTR org, LPCTSTR app, LPCTSTR filename = NULL);
bool BrowseForFolder(HWND hwnd, LPTSTR resultingPath, LPCTSTR title, LPCTSTR startingPath);

inline void GetAppDataPath(char* resultingPath, LPCTSTR org, LPCTSTR app, LPCTSTR filename = NULL)
{
    TCHAR resultingPathW[MAX_PATH];
    GetAppDataPath(resultingPathW, org, app, filename);
    pathToUtf8(resultingPath, resultingPathW);
}

inline HWND FixFont(HWND hwnd)
{
    SendMessage(hwnd, WM_SETFONT, (LPARAM)GetStockObject(DEFAULT_GUI_FONT), true);
    return hwnd;
}

inline void PumpMessages()
{
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

inline BOOL SetNewWindowText(HWND hwnd, LPCTSTR str, BOOL pump = TRUE)
{
    int len = GetWindowTextLength(hwnd);
    TCHAR orig[len + 1] = {0};
    GetWindowText(hwnd, orig, len + 1);
    if (memcmp(orig, str, sizeof(orig)))
    {
        BOOL ret = SetWindowText(hwnd, str);
        if (pump)
        {
            PumpMessages();
        }
        return ret;
    }
    else
    {
        return TRUE;
    }
}

#endif // UTIL_H
