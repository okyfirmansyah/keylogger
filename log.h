/**@file log.h
 *
 * This is part of keylogger, demo project to hook key event of a process
 * and do periodic screenshoot of screen.
 *
 * @author Oky Firmansyah <mail@okyfirmansyah.net>.
 *
 * @date Created      : Apr 03, 2017 okyfirmansyah
 */

#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED
#include <string>
#include <fstream>

/** this is implementation of mini-logging mechanism
 */

void logInit(const std::string &filename);

extern std::unique_ptr<std::ofstream> logStream;
void logTime();


static void logOut()
{

}

template<typename First, typename... T>
static void logOut(First&& first, T&&...args)
{
    *logStream<<std::forward<First>(first);
    logOut(std::forward<T>(args)...);
}



template<typename First, typename... T>
void logInfo(First&& first, T&&...args)
{
    logTime();
    *logStream<<"[Info]";
    logOut(std::forward<First>(first),std::forward<T>(args)...);
    *logStream<<std::endl;
}

template<typename First, typename... T>
void logWarning(First&& first, T&&...args)
{
    logTime();
    *logStream<<"[Warning]";
    logOut(std::forward<First>(first),std::forward<T>(args)...);
    *logStream<<std::endl;
}


template<typename First, typename... T>
void logError(First&& first, T&&...args)
{
    logTime();
    *logStream<<"[Error]";
    logOut(std::forward<First>(first),std::forward<T>(args)...);
    *logStream<<std::endl;
}


#endif // LOG_H_INCLUDED
