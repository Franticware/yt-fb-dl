#define UNICODE
#include <windows.h>

#include <shlobj.h>

#include "util.h"
#include <cstdio>

#define ID_CHECKBOX_FLAG 64

#define ID_EDIT_URL 1
#define ID_BUTTON_DOWNLOAD 2
#define ID_BUTTON_UPDATE 3
#define ID_STATIC_UPDATE_YT_DLP 4
#define ID_STATIC_UPDATE_FFMPEG 5
#define ID_CHECKBOX_EXISTING_FFMPEG (6 | ID_CHECKBOX_FLAG)
#define ID_CHECKBOX_LATEST_FFMPEG (7 | ID_CHECKBOX_FLAG)
#define ID_CHECKBOX_MP4 (8 | ID_CHECKBOX_FLAG)
#define ID_BUTTON_OPTIONS 9

#define IDI_APP 100

#define STR_ORG "Franticware"
#define STR_APP "yt-fb-dl"
#define STR_CAPTION "https://github.com/Franticware/yt-fb-dl"
#define STR_FILE_YT_DLP_EXE "yt-dlp_x86.exe"
#define STR_FILE_FFMPEG_EXE "ffmpeg.exe"
#define STR_FILE_FFMPEG_ZIP "ffmpeg.zip"

#define STR_UPDATING_YT_DLP "Updating yt-dlp... "
#define STR_UPDATING_FFMPEG "Updating ffmpeg... "
#define STR_UPDATING_DOWNLOAD_FAILED "download FAILED."
#define STR_UPDATING_UNZIPPING "unzipping."
#define STR_UPDATING_UNZIPPING_FAILED "unzipping FAILED."
#define STR_UPDATING_DONE "done."

#define STR_URL_YT_DLP_LATEST "https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp_x86.exe"

#define STR_URL_FFMPEG_LATEST "https://github.com/yt-dlp/FFmpeg-Builds/releases/download/latest/ffmpeg-master-latest-win32-gpl.zip"
#define STR_URL_FFMPEG \
    "https://github.com/yt-dlp/FFmpeg-Builds/releases/download/autobuild-2023-12-01-14-05/ffmpeg-N-112876-ga30adf9f96-win32-gpl.zip"

#define STR_PATH_FFMPEG_LATEST "ffmpeg-master-latest-win32-gpl/bin/ffmpeg.exe"
#define STR_PATH_FFMPEG "ffmpeg-N-112876-ga30adf9f96-win32-gpl/bin/ffmpeg.exe"

static HWND hwndEditUrl;
static HWND hwndButtonDownload;
static HWND hwndButtonUpdate;
static HWND hwndStaticUpdateYtDlp;
static HWND hwndStaticUpdateFfmpeg;
static HWND hwndButtonOptions;
static HWND hwndCheckboxExistingFfmpeg;
static HWND hwndCheckboxLatestFfmpeg;
static HWND hwndCheckboxMp4;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSW wc = {0};
    wc.lpszClassName = TEXT("yt-fb-dl");
    wc.hInstance = hInstance;
    wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
    wc.lpfnWndProc = WndProc;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_APP));

    RegisterClass(&wc);
    CreateWindow(wc.lpszClassName, TEXT(STR_CAPTION), WS_OVERLAPPEDWINDOW | WS_VISIBLE, 220, 220, 870, 200, 0, 0, hInstance, 0);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

class DlCallback : public IBindStatusCallback
{
public:
    DlCallback(HWND hwnd, LPCTSTR s) : hwndStatic(hwnd), str(s), lastUpdate(0) {}
    ~DlCallback() {}
    STDMETHOD(OnProgress)(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR wszStatusText)
    {
        if (ulStatusCode == BINDSTATUS_CONNECTING)
        {
            TCHAR text[MAX_PATH] = {0};
            _snwprintf(text, MAX_PATH - 1, L"%ls connecting.", str);
            SetNewWindowText(hwndStatic, text);
        }
        else if (ulStatusCode == BINDSTATUS_DOWNLOADINGDATA)
        {
            DWORD now = GetTickCount();
            const DWORD minInterval = 200;
            if (lastUpdate == 0 || now < lastUpdate || (now - lastUpdate) > minInterval)
            {
                lastUpdate = now;
                if (ulProgressMax > 0)
                {
                    double percent = ulProgress * 100.0 / ulProgressMax;
                    TCHAR text[MAX_PATH] = {0};
                    _snwprintf(text, MAX_PATH - 1, L"%ls %.0f %% downloaded.", str, percent);
                    SetNewWindowText(hwndStatic, text);
                }
                else
                {
                    double mb = ulProgress / (1024.0 * 1024.0);
                    TCHAR text[MAX_PATH] = {0};
                    _snwprintf(text, MAX_PATH - 1, L"%ls %.1f MB downloaded.", str, mb);
                    SetNewWindowText(hwndStatic, text);
                }
            }
        }
        return S_OK;
    }
    STDMETHOD(OnStartBinding)(DWORD dwReserved, IBinding __RPC_FAR* pib) { return E_NOTIMPL; }
    STDMETHOD(GetPriority)(LONG __RPC_FAR* pnPriority) { return E_NOTIMPL; }
    STDMETHOD(OnLowResource)(DWORD reserved) { return E_NOTIMPL; }
    STDMETHOD(OnStopBinding)(HRESULT hresult, LPCWSTR szError) { return E_NOTIMPL; }
    STDMETHOD(GetBindInfo)(DWORD __RPC_FAR* grfBINDF, BINDINFO __RPC_FAR* pbindinfo) { return E_NOTIMPL; }
    STDMETHOD(OnDataAvailable)(DWORD grfBSCF, DWORD dwSize, FORMATETC __RPC_FAR* pformatetc, STGMEDIUM __RPC_FAR* pstgmed) { return E_NOTIMPL; }
    STDMETHOD(OnObjectAvailable)(REFIID riid, IUnknown __RPC_FAR* punk) { return E_NOTIMPL; }
    STDMETHOD_(ULONG, AddRef)() { return 0; }
    STDMETHOD_(ULONG, Release)() { return 0; }
    STDMETHOD(QueryInterface)(REFIID riid, void __RPC_FAR* __RPC_FAR* ppvObject) { return E_NOTIMPL; }

private:
    HWND hwndStatic;
    LPCTSTR str;
    DWORD lastUpdate;
};

static void InnerUpdate(HWND hwnd)
{
    SetNewWindowText(hwndStaticUpdateYtDlp, TEXT(STR_UPDATING_YT_DLP));

    TCHAR ytdlpExePath[MAX_PATH] = {0};
    GetAppDataPath(ytdlpExePath, TEXT(STR_ORG), TEXT(STR_APP), TEXT(STR_FILE_YT_DLP_EXE));

    DlCallback pCallback0(hwndStaticUpdateYtDlp, TEXT(STR_UPDATING_YT_DLP));
    HRESULT res = URLDownloadToFile(NULL, TEXT(STR_URL_YT_DLP_LATEST), ytdlpExePath, 0, &pCallback0);
    if (res != S_OK)
    {
        SetNewWindowText(hwndStaticUpdateYtDlp, TEXT(STR_UPDATING_YT_DLP STR_UPDATING_DOWNLOAD_FAILED));
        return;
    }
    SetNewWindowText(hwndStaticUpdateYtDlp, TEXT(STR_UPDATING_YT_DLP STR_UPDATING_DONE));

    TCHAR ffmpegExePath[MAX_PATH] = {0};
    GetAppDataPath(ffmpegExePath, TEXT(STR_ORG), TEXT(STR_APP), TEXT(STR_FILE_FFMPEG_EXE));

    if (!IsDlgButtonChecked(hwnd, ID_CHECKBOX_EXISTING_FFMPEG) && FileExists(ffmpegExePath))
    {
        return;
    }

    const bool latestFfmpeg = IsDlgButtonChecked(hwnd, ID_CHECKBOX_LATEST_FFMPEG);

    TCHAR ffmpegZipPath[MAX_PATH] = {0};
    GetAppDataPath(ffmpegZipPath, TEXT(STR_ORG), TEXT(STR_APP), TEXT(STR_FILE_FFMPEG_ZIP));

    DlCallback pCallback(hwndStaticUpdateFfmpeg, TEXT(STR_UPDATING_FFMPEG));
    res = URLDownloadToFile(NULL, latestFfmpeg ? TEXT(STR_URL_FFMPEG_LATEST) : TEXT(STR_URL_FFMPEG), ffmpegZipPath, 0, &pCallback);
    if (res != S_OK)
    {
        SetNewWindowText(hwndStaticUpdateFfmpeg, TEXT(STR_UPDATING_FFMPEG STR_UPDATING_DOWNLOAD_FAILED));
        return;
    }
    SetNewWindowText(hwndStaticUpdateFfmpeg, TEXT(STR_UPDATING_FFMPEG STR_UPDATING_UNZIPPING));
    if (!unzipFile(ffmpegZipPath, latestFfmpeg ? TEXT(STR_PATH_FFMPEG_LATEST) : TEXT(STR_PATH_FFMPEG), ffmpegExePath))
    {
        SetNewWindowText(hwndStaticUpdateFfmpeg, TEXT(STR_UPDATING_FFMPEG STR_UPDATING_UNZIPPING_FAILED));
        return;
    }
    SetNewWindowText(hwndStaticUpdateFfmpeg, TEXT(STR_UPDATING_FFMPEG STR_UPDATING_DONE));
}

static void Update(HWND hwnd)
{
    EnableWindow(hwndButtonDownload, false);
    EnableWindow(hwndButtonUpdate, false);

    InnerUpdate(hwnd);

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
              FixFont(CreateWindow(TEXT("button"), TEXT("Update"), WS_VISIBLE | WS_CHILD, 5, 5, 80, 30, hwnd, (HMENU)ID_BUTTON_UPDATE, NULL, NULL));
            hwndStaticUpdateYtDlp = FixFont(CreateWindow(
              TEXT("Static"), TEXT(""), WS_CHILD | WS_VISIBLE | SS_LEFT, 90, 5, 750, 15, hwnd, (HMENU)ID_STATIC_UPDATE_YT_DLP, NULL, NULL));
            hwndStaticUpdateFfmpeg = FixFont(CreateWindow(
              TEXT("Static"), TEXT(""), WS_CHILD | WS_VISIBLE | SS_LEFT, 90, 20, 750, 15, hwnd, (HMENU)ID_STATIC_UPDATE_FFMPEG, NULL, NULL));
            hwndButtonDownload = FixFont(
              CreateWindow(TEXT("button"), TEXT("Download"), WS_VISIBLE | WS_CHILD, 5, 40, 80, 30, hwnd, (HMENU)ID_BUTTON_DOWNLOAD, NULL, NULL));
            hwndEditUrl =
              FixFont(CreateWindow(TEXT("Edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 90, 43, 750, 26, hwnd, (HMENU)ID_EDIT_URL, NULL, NULL));
            hwndButtonOptions = FixFont(CreateWindow(
              TEXT("button"), TEXT("Advanced options"), WS_VISIBLE | WS_CHILD, 5, 90, 120, 30, hwnd, (HMENU)ID_BUTTON_OPTIONS, NULL, NULL));
            hwndCheckboxExistingFfmpeg = FixFont(CreateWindow(TEXT("button"),
                                                              TEXT("Update existing ffmpeg"),
                                                              WS_CHILD | BS_CHECKBOX,
                                                              5,
                                                              90,
                                                              200,
                                                              26,
                                                              hwnd,
                                                              (HMENU)ID_CHECKBOX_EXISTING_FFMPEG,
                                                              NULL,
                                                              NULL));
            CheckDlgButton(hwnd, ID_CHECKBOX_EXISTING_FFMPEG, BST_UNCHECKED);
            hwndCheckboxLatestFfmpeg = FixFont(CreateWindow(
              TEXT("button"), TEXT("Latest ffmpeg"), WS_CHILD | BS_CHECKBOX, 5, 116, 750, 26, hwnd, (HMENU)ID_CHECKBOX_LATEST_FFMPEG, NULL, NULL));
            CheckDlgButton(hwnd, ID_CHECKBOX_LATEST_FFMPEG, BST_CHECKED);
            hwndCheckboxMp4 = FixFont(
              CreateWindow(TEXT("button"), TEXT("Prefer mp4"), WS_CHILD | BS_CHECKBOX, 210, 90, 200, 26, hwnd, (HMENU)ID_CHECKBOX_MP4, NULL, NULL));
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
                else if (id == ID_BUTTON_OPTIONS)
                {
                    ShowWindow(hwndButtonOptions, SW_HIDE);
                    ShowWindow(hwndCheckboxExistingFfmpeg, SW_SHOW);
                    ShowWindow(hwndCheckboxLatestFfmpeg, SW_SHOW);
                    ShowWindow(hwndCheckboxMp4, SW_SHOW);
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
