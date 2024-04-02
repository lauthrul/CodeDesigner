#pragma once

#include "com.h"
#include <functional>

namespace util
{
    META_ENGINE_API string enumString(const EnumMap& em, int e, const char* def = "");
    META_ENGINE_API int enumFromString(const EnumMap& em, const char* s, int def = -1);
    META_ENGINE_API string readFileContent(const char* path);
    enum ReadFileLineAction { DISCARD, TAKE, DONE };
    META_ENGINE_API bool readFileLines(vector<std::string>& lines, const char* path,
                                       std::function<ReadFileLineAction(const string& line)> filter = nullptr);

    namespace datetime
    {
        META_ENGINE_API long long nanoTimeStamp();
        META_ENGINE_API string now(const string& fmt = "%Y/%m/%d %H:%M:%S", const string& extfmt = ".%mS.%uS"); // %mS 毫秒, %uS 微秒
        META_ENGINE_API time_t str2tm(const string& str, int& ms, int& us);
        META_ENGINE_API int diff_us(const string& start, const string& end);
    }

    namespace str
    {
        template<typename ... Args>
        string format(const char* format, Args ... args)
        {
            auto size_buf = snprintf(nullptr, 0, format, args ...) + 1;
            unique_ptr<char[]> buf(new (nothrow) char[size_buf]);

            if (!buf) return string("");

            snprintf(buf.get(), size_buf, format, args ...);
            return string(buf.get(), buf.get() + size_buf - 1);
        }

        META_ENGINE_API size_t split(vector<string>& dest, const char* src, const char* patten, bool allow_empty /*= true*/);
        META_ENGINE_API size_t regex(vector<string>& dest, const char* str, const char* patten);
        META_ENGINE_API string trim(const string str, const char* patten);
        META_ENGINE_API size_t count(const string& str, const char* patten);
        META_ENGINE_API bool contains(const string& str, const list<string> patterns);
        META_ENGINE_API string join(const vector<string>& vct, const char* split);
        META_ENGINE_API string substr(const string& line, const string& startSymbol, const string& endSymbol);
    }
}
