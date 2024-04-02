#pragma once

#include <vector>
#include <list>
#include <map>
#include <memory>
#include <string>
using namespace std;


#ifdef META_ENGINE_SDK
    #ifdef EXTERN_C
        #define META_ENGINE_API extern "C" _declspec(dllexport)
    #else
        #define META_ENGINE_API _declspec(dllexport)
    #endif
#else
    #define META_ENGINE_API /*_declspec(dllimport)*/
#endif // META_ENGINE_SDK


enum MELogLevel
{
    ME_TRACE, ME_DEBUG, ME_INFO, ME_WARN, ME_ERROR, ME_CRITICAL,
};

extern META_ENGINE_API void log(int lvl, const char* file, int line, const char* func,  const char* fmt, ...);
#define LOG_TRACE(...)      log(ME_TRACE, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_DEBUG(...)      log(ME_DEBUG, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_INFO(...)       log(ME_INFO, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_WARN(...)       log(ME_WARN, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_ERROR(...)      log(ME_ERROR, __FILE__, __LINE__, __FUNCTION__,  __VA_ARGS__)
#define LOG_CRITICAL(...)   log(ME_CRITICAL, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)


#ifndef SAFE_DELETE
#define SAFE_DELETE(p) \
    if (p != NULL) \
    { \
        delete p; \
        p = NULL; \
    }
#endif


#ifndef DECALRE_SETTER_GETTER
#define DECALRE_SETTER_GETTER(T, NAME) \
    void set##NAME(T t); \
    T get##NAME() const;
#endif // !DECALRE_SETTER_GETTER

#ifndef IMPL_SETTER_GETTER
#define IMPL_SETTER_GETTER(T, SCOPE, NAME, DATA) \
    void SCOPE::set##NAME(T data) {DATA = data;} \
    T SCOPE::get##NAME() const {return DATA;}
#endif // !IMPL_SETTER_GETTER


#define UP_DECLARE(T)       using UP##T = unique_ptr<T>;
#define SP_DECLARE(T)       using SP##T = shared_ptr<T>;
#define MAKE_SHARED(T, p)   make_shared<T>(p)


using EnumMap = list<pair<int, string>>;


struct META_ENGINE_API IInterface
{
    virtual ~IInterface() {};
    virtual string toString() const { return ""; }
};
