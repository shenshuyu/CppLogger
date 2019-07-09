// PHZ
// 2018-5-15

#if defined(WIN32) || defined(_WIN32) 
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <chrono>
#include "Logger.h"
#include <stdarg.h>
#include <iostream>

using namespace log;

const char* Priority_To_String[] =
{
    "DEBUG",
    "CONFIG",
    "INFO",
    "WARNING",
    "ERROR"
};

Logger::Logger() 
    : _shutdown(false)
{
    _thread = std::thread(&Logger::run, this);
}

Logger& Logger::instance()
{
    static Logger s_logger;
    return s_logger;
}

Logger::~Logger()
{
    _shutdown = true;
    _cond.notify_all();

    _thread.join();
}

void Logger::setLogFile(char *pathname)
{
    _ofs.open(pathname);
    if (_ofs.fail()) 
    {
        std::cerr << "Failed to open logfile." << std::endl;
    }
}

void Logger::log(Priority priority, const char* __file, const char* __func, int __line, const char *fmt, ...)
{	
    char buf[2048] = {0};

    sprintf(buf, "[%s][%s:%s:%d] ", Priority_To_String[priority],  __file, __func, __line);
    va_list args;
    va_start(args, fmt);
    vsprintf(buf+strlen(buf), fmt, args);
    va_end(args);

    std::string entry(buf);
    std::unique_lock<std::mutex> lock(_mutex);	
    _queue.push(std::move(entry));
    _cond.notify_all(); 
}

void Logger::log(Priority priority, const char *fmt, ...)
{
    char buf[2048] = { 0 };

    sprintf(buf, "[%s] ", Priority_To_String[priority]);
    va_list args;
    va_start(args, fmt);
    vsprintf(buf + strlen(buf), fmt, args);
    va_end(args);

    std::string entry(buf);
    std::unique_lock<std::mutex> lock(_mutex);
    _queue.push(std::move(entry));
    _cond.notify_all();
}

void Logger::run()
{
    std::unique_lock<std::mutex> lock(_mutex);

    while(!_shutdown) 
    {
        if(!_queue.empty())
        {
            if(_ofs.is_open() && (!_shutdown))
                _ofs << "[" << localtime() << "]" 
                     << _queue.front() << std::endl;
            else
                std::cout << "[" << localtime() << "]" 
                      << _queue.front() << std::endl;
            _queue.pop();
        }
        else
        {
            _cond.wait(lock);
        }
    }
}

std::string Logger::localtime()
{
    std::ostringstream stream;
    auto now = std::chrono::system_clock::now();
    time_t tt = std::chrono::system_clock::to_time_t(now);
	
#if defined(WIN32) || defined(_WIN32)
    struct tm tm;
    localtime_s(&tm, &tt);
    stream << std::put_time(&tm, "%F %T");
#elif  defined(__linux) || defined(__linux__) 
    char buffer[200] = {0};
    std::string timeString;
    std::strftime(buffer, 200, "%F %T", std::localtime(&tt));
    stream << buffer;
#endif	
    return stream.str();
}

