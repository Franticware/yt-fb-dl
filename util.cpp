#include "util.h"

#include <cstdio>
#include <shlobj.h>

bool FileExists(LPCTSTR szPath)
{
    DWORD dwAttrib = GetFileAttributes(szPath);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void GetAppDataPath(LPTSTR resultingPath, LPCTSTR org, LPCTSTR app, LPCTSTR filename)
{
    TCHAR basePath[MAX_PATH] = {0};
    resultingPath[0] = resultingPath[MAX_PATH - 1] = 0;
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, basePath)))
    {
        _snwprintf(resultingPath, MAX_PATH - 1, L"%ls\\%ls", basePath, org);
        CreateDirectory(resultingPath, NULL);
        _snwprintf(resultingPath, MAX_PATH - 1, L"%ls\\%ls\\%ls", basePath, org, app);
        CreateDirectory(resultingPath, NULL);
        if (filename)
        {
            _snwprintf(resultingPath, MAX_PATH - 1, L"%ls\\%ls\\%ls\\%ls", basePath, org, app, filename);
        }
        else
        {
            _snwprintf(resultingPath, MAX_PATH - 1, L"%ls\\%ls\\%ls\\", basePath, org, app);
        }
    }
}

INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
    if (uMsg == BFFM_INITIALIZED)
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
    return 0;
}

bool BrowseForFolder(HWND hwnd, LPTSTR resultingPath, LPCTSTR title, LPCTSTR startingPath)
{
    resultingPath[0] = 0;

    BROWSEINFO br;
    ZeroMemory(&br, sizeof(BROWSEINFO));
    br.lpfn = BrowseCallbackProc;
    br.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    br.hwndOwner = hwnd;
    br.lpszTitle = title;
    br.lParam = (LPARAM)startingPath;

    LPITEMIDLIST pidl = NULL;
    if ((pidl = SHBrowseForFolder(&br)) != NULL)
    {
        if (SHGetPathFromIDList(pidl, resultingPath))
        {
            return true;
        }
        else
        {
            resultingPath[0] = 0;
        }
    }
    return false;
}
