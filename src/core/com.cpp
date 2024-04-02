#include "com.h"
#include <iomanip>
#include <sstream>
#include <chrono>
#include <string>
#include <mutex>
#include <cstdarg>
#include <iostream>

using namespace std;
using namespace chrono;

string time()
{
    auto now = system_clock::now();
    auto time_c = system_clock::to_time_t(now);
    auto time_tm = localtime(&time_c);
    auto duration = now.time_since_epoch();
    auto milli = duration_cast<milliseconds>(duration);
    auto micro = duration_cast<microseconds>(duration - milli);
    stringstream ss;
    ss << put_time(time_tm, "%Y/%m/%d %H:%M:%S")
       << '.' << setfill('0') << setw(3) << milli.count() % 1000
       << '.' << setfill('0') << setw(3) << micro.count() % 1000;
    return ss.str();
}

static map<int, char> sLevelTag =
{
    {ME_TRACE, 'T'},
    {ME_DEBUG, 'D'},
    {ME_INFO, 'I'},
    {ME_WARN, 'W'},
    {ME_ERROR, 'E'},
    {ME_CRITICAL, 'C'},
};

#if defined(USE_CZLOG)
#include "CzLog/CzLog.h"
void log(int lvl, const char* file, int line, const char* func, const char* fmt, ...)
{
    string str;
    va_list args;
    va_start(args, fmt);
    int len = _vscprintf_p(fmt, args) + 1;
    char* pbuf = (char*)malloc(len + 1);
    memset(pbuf, 0, len + 1);
    _vsprintf_p(pbuf, len, fmt, args);
    str = pbuf;
    free(pbuf);
    va_end(args);

    auto logid = CZLOG::Log_TestPlan;
    switch (lvl)
    {
        case ME_TRACE:
            CZLOG::CzLoggerMgr::GetLog(logid)->trace(file, line, func, str.c_str());
            break;
        case ME_DEBUG:
            CZLOG::CzLoggerMgr::GetLog(logid)->debug(file, line, func, str.c_str());
            break;
        case ME_INFO:
            CZLOG::CzLoggerMgr::GetLog(logid)->info(file, line, func, str.c_str());
            break;
        case ME_WARN:
            CZLOG::CzLoggerMgr::GetLog(logid)->warn(file, line, func, str.c_str());
            break;
        case ME_ERROR:
            CZLOG::CzLoggerMgr::GetLog(logid)->error(file, line, func, str.c_str());
            break;
        case ME_CRITICAL:
            CZLOG::CzLoggerMgr::GetLog(logid)->critical(file, line, func, str.c_str());
            break;
    }
}
#elif defined(UST_BOOST_LOG)
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/sources/severity_logger.hpp>
namespace logging = boost::log;

class LogWrapper
{
public:
    LogWrapper()
    {
        auto console_sink = logging::add_console_log();
        auto file_sink = logging::add_file_log(
                             logging::keywords::open_mode = std::ios_base::app, // 追加方式
                             logging::keywords::file_name = "log/%Y%m%d_%-2N.log", // 文件名
                             logging::keywords::rotation_size = 50 * 1024 * 1024, // 单个文件限制大小
                             logging::keywords::time_based_rotation =
                                 logging::sinks::file::rotation_at_time_point(0, 0, 0) // 每天重建
                         );
        file_sink->set_filter(logging::trivial::severity >= logging::trivial::trace);
        file_sink->locked_backend()->auto_flush(true);
        logging::core::get()->add_sink(console_sink);
        logging::core::get()->add_sink(file_sink);
    }
    logging::sources::severity_logger<logging::trivial::severity_level> logger;
} logwrapper;

void log(int lvl, const char* file, int line, const char* func, const char* fmt, ...)
{
    stringstream ss;
    ss << "[" << time() << "] " << sLevelTag[lvl] << " #" << this_thread::get_id() << " ";
    auto str = ss.str();

    va_list args;
    va_start(args, fmt);
    size_t len = (size_t)_vscprintf_p(fmt, args) + 1;
    char* pbuf = (char*)malloc(len + 1);
    if (pbuf)
    {
        memset(pbuf, 0, len + 1);
        _vsprintf_p(pbuf, len, fmt, args);
        str += pbuf;
        free(pbuf);
    }
    va_end(args);

    BOOST_LOG_SEV(logwrapper.logger, (logging::trivial::severity_level)lvl) << str;
}
#else
std::mutex logmtx;
void log(int lvl, const char* file, int line, const char* func, const char* fmt, ...)
{
    logmtx.lock();
    {
        stringstream ss;
        ss << "[" << time() << "] " << sLevelTag[lvl] << " #" << this_thread::get_id() << " ";
        auto str = ss.str();

        va_list args;
        va_start(args, fmt);
        size_t len = (size_t)_vscprintf_p(fmt, args) + 1;
        char* pbuf = (char*)malloc(len + 1);
        if (pbuf)
        {
            memset(pbuf, 0, len + 1);
            _vsprintf_p(pbuf, len, fmt, args);
            str += pbuf;
            free(pbuf);
        }
        va_end(args);

        std::cout << str << std::endl;
    }
    logmtx.unlock();
}
#endif // USE_CZLOG
