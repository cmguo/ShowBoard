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
