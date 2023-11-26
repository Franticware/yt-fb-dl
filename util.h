#ifndef UTIL_H
#define UTIL_H

#define UNICODE
#include <windows.h>

#define MAX_PATH8 1024

bool FileExists(LPCTSTR szPath);
void GetAppDataPath(LPTSTR resultingPath, LPCTSTR org, LPCTSTR app, LPCTSTR filename = NULL);
bool BrowseForFolder(HWND hwnd, LPTSTR resultingPath, LPCTSTR title, LPCTSTR startingPath);

inline void pathToUtf8(char* path8, LPCTSTR pathw)
{
    path8[MAX_PATH8 - 1] = 0;
    WideCharToMultiByte(CP_UTF8, 0, pathw, -1, path8, MAX_PATH8 - 1, NULL, NULL);
}

inline void GetAppDataPath(char* resultingPath, LPCTSTR org, LPCTSTR app, LPCTSTR filename = NULL)
{
    TCHAR resultingPathW[MAX_PATH];
    GetAppDataPath(resultingPathW, org, app, filename);
    pathToUtf8(resultingPath, resultingPathW);
}

#endif // UTIL_H
