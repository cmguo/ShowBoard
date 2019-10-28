#include <Windows.h>

BOOL CALLBACK EnumWindowsProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
    char temp[70] = {0};
    char const ** titleParts = *reinterpret_cast<char const ***>(lParam);
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

intptr_t getHwnd(char const * titleParts[])
{
    HWND hWnd = nullptr;
    ::EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&titleParts));
    hWnd = reinterpret_cast<HWND>(titleParts);
    return reinterpret_cast<intptr_t>(hWnd);
}

bool isHwndShown(intptr_t hwnd)
{
    HWND hWnd = reinterpret_cast<HWND>(hwnd);
    return ::IsWindow(hWnd);// && (::GetWindowLongA(hWnd, GWL_STYLE) & WS_VISIBLE);
}
