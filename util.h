/**@file util.h
 *
 * This is part of keylogger, demo project to hook key event of a process
 * and do periodic screenshoot of screen.
 *
 * @author Oky Firmansyah <mail@okyfirmansyah.net>.
 *
 * @date Created      : Apr 03, 2017 okyfirmansyah
 */

#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <functional>
#include <tlhelp32.h>
#include <thread>
#include <atomic>
#include "log.h"
#include "util.h"

/** implementation of some helper functions
 */

using namespace std;

/** get HWND from a PID
 */
HWND getHwndFromProcessId(DWORD processId);

/** iterate every running process, on each entry, execute func(entry)
 */
void iterateProcess(const std::function<bool(PROCESSENTRY32* entry)>& func);

/** find first process that having name procName,
  * if found, execute func(entry) and stop
  */
bool findFirstProcess(string procName, const std::function<void(PROCESSENTRY32*entry)>& func=[](PROCESSENTRY32*){});

/** find process that having name procName,
  * each encounter will call func(entry)
  */
bool findProcess(string procName, const std::function<void(PROCESSENTRY32*entry)>& func=[](PROCESSENTRY32*){});

/** take screenshot, save it to the path\YYYYMMDD_HHMMSS.bmp
  */
bool takeScreenShot(string path);

#endif // UTIL_H_INCLUDED
