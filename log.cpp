/**@file log.cpp
 *
 * This is part of keylogger, demo project to hook key event of a process
 * and do periodic screenshoot of screen.
 *
 * @author Oky Firmansyah <mail@okyfirmansyah.net>.
 *
 * @date Created      : Apr 03, 2017 okyfirmansyah
 */


#include <windows.h>
#include <winbase.h>
#include <memory>
#include <fstream>
#include <time.h>
#include "log.h"
using namespace std;

/** this is implementation of mini-logging mechanism
 */

std::unique_ptr<std::ofstream> logStream;

void logInit(const string &filename)
{
    logStream.reset(new ofstream(filename,ofstream::app));
}

void logTime()
{
    time_t timer;
    char buffer[100];
    time(&timer);
    struct tm* timeinfo = localtime (&timer);

    //strftime (buffer,sizeof(buffer),"[%F %R]",timeinfo);
    snprintf(buffer, sizeof(buffer),"[%.4d-%.2d-%.2d %.2d:%.2d:%.2d]",  1900+timeinfo->tm_year,
                                                           1+timeinfo->tm_mon,
                                                           timeinfo->tm_mday,
                                                           timeinfo->tm_hour,
                                                           timeinfo->tm_min,
                                                           timeinfo->tm_sec);
    *logStream<<buffer;
}
