#include <Windows.h>
#ifdef _DEBUG
#include <gdiplus.h>
#include <gdiplusheaders.h>
#endif

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
    char const ** temp = titleParts;
    ::EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&temp));
    if (temp != titleParts)
        hWnd = reinterpret_cast<HWND>(temp);
    return reinterpret_cast<intptr_t>(hWnd);
}

bool isWindowValid(intptr_t hwnd)
{
    HWND hWnd = reinterpret_cast<HWND>(hwnd);
    return ::IsWindow(hWnd);
}

bool isWindowShown(intptr_t hwnd)
{
    HWND hWnd = reinterpret_cast<HWND>(hwnd);
    return ::IsWindow(hWnd) && (::GetWindowLongA(hWnd, GWL_STYLE) & WS_VISIBLE);
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
    HCURSOR hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    ::SetCursor(hCursor);
}

void showCursor()
{
    ::ShowCursor(true);
}

int captureImage(intptr_t hwnd, char ** out, int * nout)
{
    HWND hWnd = reinterpret_cast<HWND>(hwnd);
    HDC hdcWindow;
    HDC hdcMemDC = NULL;
    HBITMAP hbMem = NULL;
    BITMAP bmpMem;

    // Retrieve the handle to a display device context for the client
    // area of the window.
    hdcWindow = GetDC(hWnd);

    // Create a compatible DC which is used in a BitBlt from the window DC
    hdcMemDC = CreateCompatibleDC(hdcWindow);

    if(!hdcMemDC)
    {
        MessageBox(hWnd, L"CreateCompatibleDC has failed",L"Failed", MB_OK);
        goto done;
    }

    // Get the client area for size calculation
    RECT rcClient;
    GetClientRect(hWnd, &rcClient);

    // Create a compatible bitmap from the Window DC
    hbMem = CreateCompatibleBitmap(hdcWindow, rcClient.right-rcClient.left, rcClient.bottom-rcClient.top);

    if(!hbMem)
    {
        MessageBox(hWnd, L"CreateCompatibleBitmap Failed",L"Failed", MB_OK);
        goto done;
    }

    // Select the compatible bitmap into the compatible memory DC.
    SelectObject(hdcMemDC,hbMem);

    // Bit block transfer into our compatible memory DC.
    if(!BitBlt(hdcMemDC,
               0,0,
               rcClient.right-rcClient.left, rcClient.bottom-rcClient.top,
               hdcWindow,
               0,0,
               SRCCOPY))
    {
        MessageBox(hWnd, L"BitBlt has failed", L"Failed", MB_OK);
        goto done;
    }

    // Get the BITMAP from the HBITMAP
    GetObject(hbMem,sizeof(BITMAP),&bmpMem);

    BITMAPFILEHEADER   bmfHeader;
    BITMAPINFOHEADER   bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmpMem.bmWidth;
    bi.biHeight = bmpMem.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    DWORD dwBmpSize = ((bmpMem.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpMem.bmHeight;

    // Add the size of the headers to the size of the bitmap to get the total file size
    DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    //Offset to where the actual bitmap bits start.
    bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

    //Size of the file
    bmfHeader.bfSize = dwSizeofDIB;

    //bfType must always be BM for Bitmaps
    bmfHeader.bfType = 0x4D42; //BM

    char *lpbitmap = new char[dwSizeofDIB];

    memcpy(lpbitmap, &bmfHeader, sizeof(BITMAPFILEHEADER));

    memcpy(lpbitmap + sizeof(BITMAPFILEHEADER), &bi, sizeof(BITMAPINFOHEADER));

    // Gets the "bits" from the bitmap and copies them into a buffer
    // which is pointed to by lpbitmap.
    GetDIBits(hdcWindow, hbMem, 0,
        (UINT)bmpMem.bmHeight,
        lpbitmap + bmfHeader.bfOffBits,
        (BITMAPINFO *)&bi, DIB_RGB_COLORS);

    *out = lpbitmap;
    *nout = dwSizeofDIB;

    //Clean up
done:
    DeleteObject(hbMem);
    DeleteObject(hdcMemDC);
    ReleaseDC(hWnd,hdcWindow);

    return 0;
}

bool saveGdiImage(char* data, int size, char** out, int * nout)
{
#ifdef _DEBUG
    static ULONG_PTR token = 0;
    if (token == 0) {
        Gdiplus::GdiplusStartupInput input;
        Gdiplus::GdiplusStartupOutput output;
        Gdiplus::GdiplusStartup(&token, &input, &output);
    }
    IStream *from = nullptr;
    ::CreateStreamOnHGlobal(nullptr, true, &from);
    from->Write(data, static_cast<ULONG>(size), nullptr);
    from->Seek({{0, 0}}, STREAM_SEEK_SET, nullptr);
    Gdiplus::Image *image = Gdiplus::Image::FromStream(from);
    if (!image)
        return false;
    from->Release();
    IStream *to = nullptr;
    ::CreateStreamOnHGlobal(nullptr, true, &to);
//    image/bmp  : {557cf400-1a04-11d3-9a73-0000f81ef32e}
//    image/jpeg : {557cf401-1a04-11d3-9a73-0000f81ef32e}
//    image/gif  : {557cf402-1a04-11d3-9a73-0000f81ef32e}
//    image/tiff : {557cf405-1a04-11d3-9a73-0000f81ef32e}
//    image/png  : {557cf406-1a04-11d3-9a73-0000f81ef32e}
    CLSID pngClsid = {0x557cf406, 0x1a04, 0x11d3, {0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e}};
    //Gdiplus::GetEncoderClsid(L"image/png", &pngClsid);
    image->Save(to, &pngClsid, nullptr);
    delete image;
    ULARGE_INTEGER end;
    to->Seek({{0, 0}}, STREAM_SEEK_END, &end);
    to->Seek({{0, 0}}, STREAM_SEEK_SET, nullptr);
    *nout = static_cast<int>(end.LowPart);
    *out = new char[static_cast<unsigned int>(*nout)];
    to->Read(*out, static_cast<ULONG>(*nout), nullptr);
    to->Release();
#endif
    return true;
}
