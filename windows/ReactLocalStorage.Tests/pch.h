// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。
#pragma once

#include "CppUnitTest.h"

// Add necessary headers for your tests
#include <string>
#include <optional>
#include <vector>
#include <map>
#include <stdexcept>
#include <algorithm>
#include <random>
//#include <thread>   // For std::this_thread
//#include <chrono>   // For std::chrono
// Windows API
#include <windows.h>
#include <wincrypt.h>
#include <bcrypt.h>
#include "V2rayConfigWin.h"
#include "V2rayManager.h"
