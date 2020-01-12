#include <Windows.h>
#include <gdiplus.h>
#include <gdiplusheaders.h>

BOOL CALLBACK EnumWindowsProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
    char temp[256] = {0};
    unsigned long pid = 0;
    char const ** titleParts = *reinterpret_cast<char const ***>(lParam);
    ::GetWindowThreadProcessId(hwnd, &pid);
    if (pid == ::GetCurrentProcessId()) // avoid deadlock
        return TRUE;
    ::GetWindowTextA(hwnd, temp, 255);
    char * p = temp;
    while (*titleParts && (p = strstr(p, *titleParts))) {
        ++titleParts;
    }
    if (*titleParts)
        return TRUE;
    *reinterpret_cast<HWND*>(lParam) = hwnd;
    return FALSE;
}

intptr_t findWindow(char const * titleParts[])
{
    HWND hWnd = nullptr;
    ::EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&titleParts));
    hWnd = reinterpret_cast<HWND>(titleParts);
    return reinterpret_cast<intptr_t>(hWnd);
}

bool isWindowShown(intptr_t hwnd)
{
    HWND hWnd = reinterpret_cast<HWND>(hwnd);
    return ::IsWindow(hWnd);// && (::GetWindowLongA(hWnd, GWL_STYLE) & WS_VISIBLE);
}

void showWindow(intptr_t hwnd)
{
    HWND hWnd = reinterpret_cast<HWND>(hwnd);
    ::SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0,
                   SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
}

void hideWindow(intptr_t hwnd)
{
    HWND hWnd = reinterpret_cast<HWND>(hwnd);
    ::SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0,
                   SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);
}

void setWindowAtTop(intptr_t hwnd)
{
    HWND hWnd = reinterpret_cast<HWND>(hwnd);
    ::SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void closeWindow(intptr_t hwnd)
{
    HWND hWnd = reinterpret_cast<HWND>(hwnd);
    ::CloseWindow(hWnd);
}

void attachWindow(intptr_t hwnd, intptr_t hwndParent, int left, int top)
{
    HWND hWndParent = reinterpret_cast<HWND>(hwndParent);
    HWND hWnd = reinterpret_cast<HWND>(hwnd);
    LONG style = ::GetWindowLongA(hWnd, GWL_STYLE);
    style |= WS_CHILDWINDOW;
    style &= ~WS_POPUP;
    style &= ~WS_SYSMENU;
    style &= ~WS_MAXIMIZEBOX;
    style &= ~WS_MINIMIZEBOX;
    ::SetWindowLongA(hWnd, GWL_STYLE, style);
    ::SetParent(hWnd, hWndParent);
    if (left < 0 || top < 0) {
        RECT rect;
        ::GetWindowRect(hWndParent, &rect);
        if (left < 0)
            left += rect.right - rect.left;
        if (top < 0)
            top += rect.bottom - rect.top;
    }
    ::SetWindowPos(hWnd, HWND_TOP, left, top, 0, 0, SWP_NOSIZE);
    //::ShowWindow(hWnd, SW_SHOWNORMAL);
    //::SetActiveWindow(hWndParent);
    ::SetFocus(hWndParent);
}

void moveChildWindow(intptr_t hwnd, int dx, int dy)
{
    HWND hWnd = reinterpret_cast<HWND>(hwnd);
    RECT rect;
    ::GetWindowRect(hWnd, &rect);
    ::SetWindowPos(hWnd, HWND_TOP, rect.left + dx, rect.top + dy,
                   0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void setArrowCursor()
{
    SetCursor(LoadCursor(GetModuleHandle(nullptr), IDC_ARROW));
}

bool saveGdiImage(char* data, int size, wchar_t * file)
{
    /*
    WCHAR gdiPath[1024];
    WCHAR * l = wcsstr(file, L".png");
    size_t n = static_cast<size_t>(l - file);
    wcsncpy_s(gdiPath, file, n);
    wcscpy_s(gdiPath + n, 1024 - n, L".gdip");
    HANDLE f = ::CreateFileW(gdiPath, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, 0, nullptr);
    DWORD bytesWriten = static_cast<DWORD>(size);
    ::WriteFile(f, data, bytesWriten, &bytesWriten, nullptr);
    ::CloseHandle(f);
    Gdiplus::Image * image = Gdiplus::Image::FromFile(gdiPath);
    if (!image)
        return false;
    CLSID pngClsid = {0x557cf406, 0x1a04, 0x11d3, {0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e}};
    //Gdiplus::GetEncoderClsid(L"image/png", &pngClsid);
    image->Save(file, &pngClsid, nullptr);
    delete image;
    */
    return true;
}
