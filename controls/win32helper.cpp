#include <Windows.h>

BOOL CALLBACK EnumWindowsProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
    char temp[70] = {0};
    unsigned long pid = 0;
    char const ** titleParts = *reinterpret_cast<char const ***>(lParam);
    ::GetWindowThreadProcessId(hwnd, &pid);
    if (pid == ::GetCurrentProcessId()) // avoid deadlock
        return TRUE;
    ::GetWindowTextA(hwnd, temp, 70);
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
    ::ShowWindow(hWnd, SW_SHOWNORMAL);
}

void moveChildWindow(intptr_t hwnd, int dx, int dy)
{
    HWND hWnd = reinterpret_cast<HWND>(hwnd);
    RECT rect;
    ::GetWindowRect(hWnd, &rect);
    ::SetWindowPos(hWnd, HWND_TOP, rect.left + dx, rect.top + dy,
                   0, 0, SWP_NOSIZE | SWP_NOZORDER);
}
