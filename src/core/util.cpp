#include "util.h"
#include <chrono>
#include <regex>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace util
{
    string enumString(const EnumMap& em, int e, const char* def /*= ""*/)
    {
        for (auto item : em)
        {
            if (item.first == e)
                return item.second;
        }
        return def;
    }

    int enumFromString(const EnumMap& em, const char* s, int def /*= -1*/)
    {
        for (auto item : em)
        {
            if (item.second == s)
                return item.first;
        }
        return def;
    }

    string readFileContent(const char* path)
    {
        ifstream file(path, ios::binary | ios::ate); // 打开文件，并移动到文件末尾
        streampos size = file.tellg(); // 获取文件大小
        file.seekg(0, ios::beg); // 回到文件开头
        vector<char> buffer((int)size); // 创建足够大的缓冲区
        file.read(buffer.data(), size);
        file.close(); // 关闭文件
        string content(buffer.begin(), buffer.end());
        return content;
    }

    bool readFileLines(vector<std::string>& lines, const char* path, std::function<ReadFileLineAction(const string& line)> filter)
    {
        std::ifstream file(path);
        if (file.is_open())
        {
            std::string line;
            while (std::getline(file, line))
            {
                auto action = DISCARD;
                if (!filter || (action = filter(line)) == TAKE)
                    lines.push_back(line);
                if (action == DONE)
                    break;
            }
            file.close();
            return true;
        }
        return false;
    }

    namespace datetime
    {
        long long nanoTimeStamp()
        {
            chrono::system_clock::time_point now = chrono::system_clock::now();
            return chrono::duration_cast<chrono::nanoseconds>(now.time_since_epoch()).count();
        }

        string now(const string& fmt /*= "%Y/%m/%d %H:%M:%S"*/, const string& extfmt /*= ".%mS.%uS"*/) // %mS 毫秒, %uS 微秒
        {
            auto now = chrono::system_clock::now();
            auto time_c = chrono::system_clock::to_time_t(now);
            struct tm time_tm;
            localtime_s(&time_tm, &time_c);

            stringstream ss;
            ss << put_time(&time_tm, fmt.c_str());

            if (!extfmt.empty())
            {
                auto duration = now.time_since_epoch();
                auto milli = chrono::duration_cast<chrono::milliseconds>(duration);
                auto micro = chrono::duration_cast<chrono::microseconds>(duration - milli);

                string ext(extfmt);
                auto pos = ext.find("%mS");
                if (pos >= 0)
                    ext.replace(pos, 3, str::format("%d", milli.count() % 1000).c_str());
                pos = fmt.find("%uS");
                if (pos >= 0)
                    ext.replace(pos, 3, str::format("%d", micro.count() % 1000).c_str());
                ss << ext;
            }

            return ss.str();
        }

        time_t str2tm(const string& str, int& ms, int& us)
        {
            struct tm tm_;
            int year, month, day, hour, minute, second;
            sscanf_s(str.c_str(), "%d/%d/%d %d:%d:%d.%d.%d",
                     &year, &month, &day, &hour, &minute, &second, &ms, &us);
            tm_.tm_year = year - 1900;
            tm_.tm_mon = month - 1;
            tm_.tm_mday = day;
            tm_.tm_hour = hour;
            tm_.tm_min = minute;
            tm_.tm_sec = second;
            tm_.tm_isdst = 0;
            time_t stamp = mktime(&tm_);
            return stamp;
        }

        int diff_us(const string& start, const string& end)
        {
            int ms1, us1, ms2, us2;
            auto tm1 = str2tm(start, ms1, us1);
            auto tm2 = str2tm(end, ms2, us2);
            auto diff = (int)((tm2 - tm1) * 1e6 + (ms2 - ms1) * 1e3 + (us2 - us1));
            return diff;
        }
    }

    namespace str
    {
        size_t split(vector<string>& dest, const char* src, const char* patten, bool allow_empty /*= true*/)
        {
            if (src == nullptr || src[0] == 0) return 0;
            if (patten == nullptr || patten[0] == 0)
            {
                dest.push_back(src);
                return dest.size();
            }

#define push(str) \
    if (strlen(str) == 0 && !allow_empty); else dest.push_back(str);

            char* pbuff = NULL;
            const char* last_pos = src;
            const char* p = last_pos;
            size_t size = 0;
            while (p != NULL)
            {
                p = strstr(last_pos, patten);
                if (p != NULL)
                {
                    size = p - last_pos;
                    pbuff = new char[size + 1];
                    memset(pbuff, 0, size + 1);
                    strncpy_s(pbuff, size + 1, last_pos, size);
                    push(pbuff);
                    delete[] pbuff;
                    last_pos = p + strlen(patten);
                    if (last_pos[0] == 0) push(last_pos);
                }
            }
            if (last_pos[0] != 0) push(last_pos);

            return dest.size();
        }

        size_t regex(vector<string>& dest, const char* str, const char* patten)
        {
            dest.clear();
            std::string source(str);
            std::smatch match;
            std::regex pattern(patten);
            if (!std::regex_search(source, match, pattern))
                return 0;

            for (size_t i = 1; i < match.size(); i++)
                dest.push_back(match[i].str());
            return dest.size();
        }

        string trim(const string str, const char* patten)
        {
            auto wsfront = str.find_first_not_of(patten);
            if (wsfront == string::npos) return "";
            auto wsback = str.find_last_not_of(patten);
            return str.substr(wsfront, (wsback - wsfront + 1));
        }

        size_t count(const string& str, const char* patten)
        {
            auto src = str;

            size_t cnt = 0;
            size_t pos = 0;
            while ((pos = str.find(patten, pos == 0 ? pos : pos + strlen(patten))) != string::npos)
                cnt++;
            return cnt;
        }

        bool contains(const string& str, const list<string> patterns)
        {
            for (const auto& pattern : patterns)
            {
                if (pattern.empty()) continue;
                if (str.find(pattern) == string::npos) return false;
            }
            return true;
        }

        string join(const vector<string>& vct, const char* split)
        {
            string str;
            for (const auto& v : vct)
                str += v + split;
            return str.substr(0, str.length() - strlen(split));
        }

        string substr(const string& line, const string& startSymbol, const string& endSymbol)
        {
            auto pos1 = line.find(startSymbol) + startSymbol.length();
            auto pos2 = line.find(endSymbol, pos1);
            return line.substr(pos1, pos2 - pos1);
        }
    }
}