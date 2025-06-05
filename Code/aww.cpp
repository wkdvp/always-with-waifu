// aww.cpp
//
// waifu always with you
// illust by Harido (@haridodesu)
//
// to compile, use the following command:
// g++ aww.cpp -o aww.exe -lgdiplus -lshell32 -mwindows -lwinmm

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <gdiplus.h>
#include <shellapi.h>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <filesystem>
#include <ctime>
#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "winmm.lib")

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001

using namespace Gdiplus;

LRESULT CALLBACK WndProcMain(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WndProcImage(HWND, UINT, WPARAM, LPARAM);
int WINAPI showImage(HINSTANCE, int, int, Gdiplus::Image*);
int openWindow();
void killWindow();
void newWind();

struct imageAndVoice
{
    std::wstring image;
    std::wstring voice;
    imageAndVoice(const std::wstring i, const std::wstring& v)
        : image(i), voice(v)
    {
    }
};

ULONG_PTR gdiplusToken;
HMENU hTrayMenu;
NOTIFYICONDATAA nid;
HINSTANCE Instance;
Gdiplus::Image* image = nullptr;
UINT width, height;
int posx, posy, cmdShow;
LPCWSTR soureDir;
LPWSTR defaultImageDir;
std::vector<imageAndVoice> src;
std::wstring WAIFU;

LPWSTR joinLPWSTRPath(LPCWSTR a, LPCWSTR b)
{
    // but it can join LPWSTR and LPCWSTR
    LPWSTR res = new wchar_t[wcslen(a) + wcslen(b) + 2];
    wcscpy(res, a);
    wcscat(res, L"/");
    wcscat(res, b);
    return res;
}

LPWSTR joinLPWSTRPath(LPCWSTR a, LPCWSTR b, LPCWSTR c)
{
    // but it can join LPWSTR and LPCWSTR
    LPWSTR res = new wchar_t[wcslen(a) + wcslen(b) + wcslen(c) + 3];
    wcscpy(res, a);
    wcscat(res, L"/");
    wcscat(res, b);
    wcscat(res, L"/");
    wcscat(res, c);
    return res;
}

void setOption()
{
    // 이미지 관련 변수 설정
    std::ifstream settingFile("E:\\codeWithWaifu\\always-with-waifu\\Always_with_waifu\\setting.txt");
    if (settingFile.fail())
    {
        MessageBoxA(nullptr, "setting.txt missing", "error", MB_OK);
        return;
    }

    // read setting.txt and set option
    std::string line;
    std::getline(settingFile, line);
    std::vector<std::string> config;
    std::stringstream ss(line);
    std::string a;
    while (std::getline(ss, a, ','))
    {
        config.push_back(a);
    }

    // set waifu
    std::getline(settingFile, line);
    WAIFU = std::wstring(line.begin(), line.end());
    settingFile.close();
    soureDir = joinLPWSTRPath(L"E:\\codeWithWaifu\\always-with-waifu\\Always_with_waifu\\source", WAIFU.c_str());

    // read config.txt
    std::wstring configDir(joinLPWSTRPath(soureDir, L"config.txt"));
    std::wifstream configFile;
    configFile.open(a.assign(configDir.begin(), configDir.end()));
    if (configFile.fail())
    {
        MessageBoxA(nullptr, "config.txt is missing", "error", MB_OK);
        return;
    }
    std::wstring wline;
    std::getline(configFile, wline); // get default waifu image
    defaultImageDir = joinLPWSTRPath(soureDir, L"image", wline.c_str());
    std::wstringstream wss;
    std::wstring Itoken, Vtoken;
    while (std::getline(configFile, wline))
    {
        wss.clear();
        wss.str(wline);
        wss >> Itoken;
        wss >> Vtoken;
        src.emplace_back(Itoken, Vtoken);
    }
    configFile.close();
    // set option
    // MessageBoxW(nullptr, defaultImageDir, soureDir, MB_OK);
    image = new Gdiplus::Image(defaultImageDir);
    // delete[] defaultImageDir;
    width = image->GetWidth() * (UINT)stoul(config[0], nullptr, 10) / 100;
    height = image->GetHeight() * (UINT)stoul(config[1], nullptr, 10) / 100;
    posx = (GetSystemMetrics(SM_CXSCREEN) - width) * (int)stoul(config[2], nullptr, 10) / 100;
    posy = (int)(GetSystemMetrics(SM_CYSCREEN) - height) * (int)stoul(config[3], nullptr, 10) / 100;
}

void callVoice(const wchar_t* v)
{
    wchar_t voicecall[256];
    wsprintfW(voicecall, L"Open \"E:\\codeWithWaifu\\always-with-waifu\\Always_with_waifu\\source/%s/voice/%s\" Type MPEGVideo Alias theMP3", WAIFU.c_str(), v);
    mciSendStringW(voicecall, NULL, 0, 0);
    mciSendStringW(L"Play theMP3 Wait", NULL, 0, 0);
    mciSendStringW(L"Close theMP3", NULL, 0, 0);
    HWND a = FindWindowA("awwtmpWindow", nullptr);
    PostMessage(a, WM_CLOSE, 0, 0);
}

void newWind()
{
    HWND a = FindWindowW(L"awwtmpWindow", nullptr);
    if (a)
    return;
    srand(time(NULL));
    int i = rand() % src.size();
    LPWSTR tmpPath = joinLPWSTRPath(soureDir, L"image", src[i].image.c_str());
    Gdiplus::Image* tmpImage = new Gdiplus::Image(tmpPath);
    delete[] tmpPath;
    std::thread(callVoice, src[i].voice.c_str()).detach();
    showImage(Instance, cmdShow, TRUE, tmpImage);
    delete tmpImage;
    // image = nullptr;
}

int WINAPI showImage(HINSTANCE hInstance, int nCmdShow, int isTmp, Gdiplus::Image* a)
{
    // delete image;
    image = a;

#ifdef T_TEST
    /*
    for (int i = 0; i < argc; ++i) {
        MessageBoxW(nullptr, argv[i], L"argv", MB_OK);
    }

    // debug outputs
    */
    wchar_t m[256];
    swprintf(m, 256, L" posy: %d", posy);
    MessageBoxW(nullptr, m, L"오류", MB_OK);
    // posy = 50;
#endif
    Status st = image->GetLastStatus();
    if (!image || st != Ok)
    {
        /*
        wchar_t msg[256];
        swprintf(msg, 256, L"error code: %d\nargc: %d argv[n-1]: %s", st, argc, argv[argc - 1]);
        MessageBoxW(nullptr, msg, L"오류", MB_OK);
        GdiplusShutdown(gdiplusToken);
        */
        MessageBoxW(nullptr, L"fail to load image. Please check your source folder", L"error", MB_OK);
        killWindow();
        exit(-1);
    }
    // hide awwWindow
    // 윈도우 클래스 등록
    LPCWSTR className = isTmp ? L"awwtmpWindow" : L"awwWindow";
    // 투명 + 테두리 없는 윈도우 생성
    HWND hwnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW, //| WS_EX_NOACTIVATE | WS_EX_TRANSPARENT
        className, nullptr,
        WS_POPUP,
        posx, posy, width, height,
        nullptr, nullptr, Instance, nullptr);
        
        ShowWindow(hwnd, nCmdShow);
        UpdateWindow(hwnd);
        HWND awindow = FindWindowW(L"awwWindow", nullptr);
        if (a && isTmp)
        {
            ShowWindow(awindow, SW_HIDE);
        }
    return 0;
}

LRESULT CALLBACK WndProcImage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        // 투명 비트맵 생성
        HDC hdcScreen = GetDC(nullptr);
        HDC hdcMem = CreateCompatibleDC(hdcScreen);

        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -((LONG)height); // top-down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        void* pvBits = nullptr;
        HBITMAP hBitmap = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pvBits, nullptr, 0);
        if (!hBitmap)
        {
            MessageBoxA(nullptr, "bitmap error", "error", MB_OK);
        }
        HBITMAP hOld = (HBITMAP)SelectObject(hdcMem, hBitmap);
        // 그리기
        Gdiplus::Graphics g(hdcMem);
        g.SetCompositingMode(Gdiplus::CompositingMode::CompositingModeSourceOver);
        g.Clear(Gdiplus::Color(0, 0, 255, 255)); // 완전 투명
        g.DrawImage(image, 0, 0, width, height);

        // 레이어드 윈도우 업데이트
        SIZE size = { (LONG)width, (LONG)height };
        POINT ptSrc = { 0, 0 };
        POINT ptWinPos = { posx, posy };
        BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

        UpdateLayeredWindow(hwnd, hdcScreen, &ptWinPos, &size, hdcMem, &ptSrc, 0, &blend, ULW_ALPHA);

        // 정리
        SelectObject(hdcMem, hOld);
        DeleteDC(hdcMem);
        ReleaseDC(nullptr, hdcScreen);
        DeleteObject(hBitmap);
        break;
    }

    case WM_LBUTTONDOWN:
        // PostMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        // MessageBoxW(nullptr, L"눌림", L"예", MB_OK);
    {
        newWind();
        break;
    }

    case WM_DESTROY:
    {
        wchar_t cls[32];
        GetClassNameW(hwnd, cls, _countof(cls));
        // MessageBoxW(nullptr, cls, L"aww", MB_OK);
        if (wcscmp(cls, L"awwtmpWindow") == 0)
        {
            HWND hMain = FindWindowW(L"awwWindow", nullptr);
            if (hMain)
            {
                //delete image;
                //image = new Gdiplus::Image(defaultImageDir);
                ShowWindow(hMain, SW_SHOW);
                InvalidateRect(hMain, nullptr, FALSE);
                UpdateWindow(hMain);
            }
        }
        else if (wcscmp(cls, L"awwWindow") == 0)
        {
            // 메인 창이 닫혔을 때 → 앱 완전 종료
            // PostQuitMessage(0);
        }
        break;
    }
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int openWindow()
{
    // char cmds[] = "/C viewer.exe ./source/skm/image/0.png 25 25 50 50 100";
    // ShellExecuteA(NULL, "open", "cmd.exe", cmds, NULL, SW_HIDE);
    // showImage(Instance, cmdShow);
    return 0;
}

void killWindow()
{
    HWND a;
    // int k = 0, s = 0, d = 0;

    a = FindWindowA("awwtmpWindow", nullptr);
    if (a)
    {
        PostMessage(a, WM_CLOSE, 0, 0);
        // d++;
    }
    a = FindWindowA("awwWindow", nullptr);
    if (a)
    {
        // MessageBoxA(nullptr, "window close", "close", MB_OK);
        PostMessage(a, WM_CLOSE, 0, 0);
        // s++;
    }
    a = FindWindowA("awwMain", nullptr);
    if (a)
    {
        PostMessage(a, WM_CLOSE, 0, 0);
        // k++;
    }

    // std::cout << "M: " << k << "\nW: " << s << "\nT: " << d << std::endl;
}

LRESULT CALLBACK WndProcMain(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_TRAYICON:
        if (lParam == WM_RBUTTONDOWN)
        {
            POINT p;
            GetCursorPos(&p);
            SetForegroundWindow(hwnd);
            TrackPopupMenu(hTrayMenu, TPM_RIGHTBUTTON, p.x, p.y, 0, hwnd, NULL);
        }
        break;
    case WM_COMMAND:
        killWindow();
        PostQuitMessage(0);
        break;
    case WM_DESTROY:

        wchar_t cls[32];
        GetClassNameW(hwnd, cls, _countof(cls));
        MessageBoxW(nullptr, cls, L"Main", MB_OK);
        Shell_NotifyIconA(NIM_DELETE, &nid);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    HWND old = FindWindowA("awwMain", nullptr);
    if (old)
    {
        MessageBoxA(nullptr, "AWW is already running!", "You are trying to have an affair!", MB_OK);
        return 0;
    }
    Instance = hInstance;
    cmdShow = nCmdShow;
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WndProcMain;
    wc.hInstance = hInstance;
    wc.lpszClassName = "awwMain";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassA(&wc);

    WNDCLASSA wcImage = {};
    wcImage.lpfnWndProc = WndProcImage;
    wcImage.hInstance = hInstance;
    wcImage.lpszClassName = "awwWindow";
    wcImage.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassA(&wcImage);
    wcImage.lpszClassName = "awwtmpWindow";
    RegisterClassA(&wcImage);

    HWND hwnd = CreateWindowExA(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW,
        wc.lpszClassName, nullptr,
        WS_POPUP,
        0, 0, 0, 0,
        nullptr, nullptr, hInstance, nullptr);

    // MessageBoxW(nullptr, soureDir, L"wow", MB_OK);
    setOption();
    // Gdiplus::Image* di = new Gdiplus::Image(soureDir);   // 기본 이미지 설정
    // Image* di = new Image(L"./source/skm/image/0.png");   // 기본 이미지 설정
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // open tray icon
    ZeroMemory(&nid, sizeof(nid));
    nid.cbSize = sizeof(NOTIFYICONDATAA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;

    HICON hIcon = (HICON)LoadImageA(
        NULL,
        "syachi.ico",
        IMAGE_ICON,
        0, 0,
        LR_LOADFROMFILE | LR_DEFAULTSIZE);
    nid.hIcon = hIcon;
    strcpy(nid.szTip, "aww");
    Shell_NotifyIconA(NIM_ADD, &nid);

    // create tray menu
    hTrayMenu = CreatePopupMenu();
    AppendMenuA(hTrayMenu, MF_STRING, ID_TRAY_EXIT, "종료");
    showImage(Instance, cmdShow, FALSE, image);
    // delete di;
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(gdiplusToken);
    delete[] soureDir;
    return 0;
}