#include <Windows.h>

BOOL CALLBACK EnumWindowsProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
    char temp[70] = {0};
    char const ** titleParts = reinterpret_cast<char const **>(lParam);
    ::GetWindowTextA(hwnd, temp, 70);
    while (*titleParts && strstr(temp, *titleParts)) {
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
    ::EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(titleParts));
    return reinterpret_cast<intptr_t>(hWnd);
}

