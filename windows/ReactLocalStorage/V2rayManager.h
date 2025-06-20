#pragma once

#include <windows.h>
#include <string>
#include <stdexcept>
#include <functional>
#include <vector>

// 1. 包含 CGo 库的头文件，以获取函数和类型的定义
//    请确保这个路径相对于 V2rayManager.h 是正确的
extern "C" {
#include "libv2ray/include/libv2ray.h"
}

class V2rayManager {
public:
    // 2. 为需要从 DLL 中调用的每个函数定义一个函数指针类型别名
    //    这些别名完全匹配 libv2ray.h 中的函数签名
    using SetCallbacksFunc = void(*)(Setup_fn, Prepare_fn, Shutdown_fn, Protect_fn, OnEmitStatus_fn);
    using CreateV2RayPointFunc = int(*)(GoUint8);
    using StartV2RayPointFunc = char*(*)(int, GoUint8, char*, char*);
    using StopV2RayPointFunc = char*(*)(int);
    using FreeV2RayPointFunc = void(*)(int);
    using QueryStatsFunc = long long int(*)(int, char*, char*);
    using MeasureDelayFunc = long long int(*)(int, char*);
    using InitV2EnvFunc = void(*)(char*, char*);
    using CheckVersionXFunc = char*(*)();
    using FreeCStringFunc = void(*)(char*);

public:
    V2rayManager() {
        // 3. 在构造函数中加载 DLL
        m_dllHandle = LoadLibrary(L"libv2ray.dll");
        if (!m_dllHandle) {
            throw std::runtime_error("Failed to load libv2ray.dll. Make sure it's in the output directory.");
        }

        // 4. 获取每个函数的地址，并存到成员变量中
        m_setCallbacks = (SetCallbacksFunc)GetProcAddress(m_dllHandle, "SetCallbacks");
        m_createV2RayPoint = (CreateV2RayPointFunc)GetProcAddress(m_dllHandle, "CreateV2RayPoint");
        m_startV2RayPoint = (StartV2RayPointFunc)GetProcAddress(m_dllHandle, "StartV2RayPoint");
        m_stopV2RayPoint = (StopV2RayPointFunc)GetProcAddress(m_dllHandle, "StopV2RayPoint");
        m_freeV2RayPoint = (FreeV2RayPointFunc)GetProcAddress(m_dllHandle, "FreeV2RayPoint");
        m_queryStats = (QueryStatsFunc)GetProcAddress(m_dllHandle, "QueryStats");
        m_measureDelay = (MeasureDelayFunc)GetProcAddress(m_dllHandle, "MeasureDelay");
        m_initV2Env = (InitV2EnvFunc)GetProcAddress(m_dllHandle, "InitV2Env");
        m_checkVersionX = (CheckVersionXFunc)GetProcAddress(m_dllHandle, "CheckVersionX");
        m_freeCString = (FreeCStringFunc)GetProcAddress(m_dllHandle, "FreeCString");

        // 5. 检查所有函数地址是否都获取成功
        if (!m_setCallbacks || !m_createV2RayPoint || !m_startV2RayPoint || !m_stopV2RayPoint ||
            !m_freeV2RayPoint || !m_queryStats || !m_measureDelay || !m_initV2Env ||
            !m_checkVersionX || !m_freeCString) {
            FreeLibrary(m_dllHandle); // 释放已加载的库
            throw std::runtime_error("Failed to get one or more function addresses from libv2ray.dll");
        }
    }

    ~V2rayManager() {
        // 6. 在析构函数中释放 DLL 句柄
        if (m_dllHandle) {
            FreeLibrary(m_dllHandle);
        }
    }

    // 7. 提供 C++ 风格的公共方法来调用这些函数
    
    void SetCallbacks(Setup_fn s, Prepare_fn p, Shutdown_fn sh, Protect_fn pr, OnEmitStatus_fn o) {
        m_setCallbacks(s, p, sh, pr, o);
    }

    int CreateV2RayPoint(bool adns) {
        return m_createV2RayPoint(adns ? 1 : 0);
    }

    // [重要] 封装了内存管理：返回 std::string，并自动调用 FreeCString
    std::string StartV2RayPoint(int handle, bool prefIPv6, const std::string& domainName, const std::string& configFileContent) {
        char* result_c = m_startV2RayPoint(handle, prefIPv6 ? 1 : 0, (char*)domainName.c_str(), (char*)configFileContent.c_str());
        return handleCharPointerResult(result_c);
    }

    // [重要] 封装了内存管理
    std::string StopV2RayPoint(int handle) {
        char* result_c = m_stopV2RayPoint(handle);
        return handleCharPointerResult(result_c);
    }

    void FreeV2RayPoint(int handle) {
        m_freeV2RayPoint(handle);
    }

    long long QueryStats(int handle, const std::string& tag, const std::string& direct) {
        return m_queryStats(handle, (char*)tag.c_str(), (char*)direct.c_str());
    }

    long long MeasureDelay(int handle, const std::string& url) {
        return m_measureDelay(handle, (char*)url.c_str());
    }

    void InitV2Env(const std::string& envPath, const std::string& key) {
        m_initV2Env((char*)envPath.c_str(), (char*)key.c_str());
    }

    // [重要] 封装了内存管理
    std::string CheckVersionX() {
        char* result_c = m_checkVersionX();
        return handleCharPointerResult(result_c);
    }

private:
    // 辅助函数，用于处理返回 char* 的函数，确保内存被正确释放
    std::string handleCharPointerResult(char* c_str) {
        if (c_str == nullptr) {
            return "";
        }
        std::string result(c_str);
        m_freeCString(c_str); // 关键：调用 CGo 库的函数来释放内存
        return result;
    }

private:
    HMODULE m_dllHandle = nullptr; // DLL 句柄

    // 用于存储函数指针的成员变量
    SetCallbacksFunc m_setCallbacks = nullptr;
    CreateV2RayPointFunc m_createV2RayPoint = nullptr;
    StartV2RayPointFunc m_startV2RayPoint = nullptr;
    StopV2RayPointFunc m_stopV2RayPoint = nullptr;
    FreeV2RayPointFunc m_freeV2RayPoint = nullptr;
    QueryStatsFunc m_queryStats = nullptr;
    MeasureDelayFunc m_measureDelay = nullptr;
    InitV2EnvFunc m_initV2Env = nullptr;
    CheckVersionXFunc m_checkVersionX = nullptr;
    FreeCStringFunc m_freeCString = nullptr;
};
