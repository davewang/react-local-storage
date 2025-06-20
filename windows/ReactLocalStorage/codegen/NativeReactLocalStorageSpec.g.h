
/*
 * This file is auto-generated from a NativeModule spec file in js.
 *
 * This is a C++ Spec class that should be used with MakeTurboModuleProvider to register native modules
 * in a way that also verifies at compile time that the native module matches the interface required
 * by the TurboModule JS spec.
 */
#pragma once
// clang-format off

// #include "NativeReactLocalStorageDataTypes.g.h" before this file to use the generated type definition
#include <NativeModules.h>
#include <tuple>

namespace ReactLocalStorageCodegen {

inline winrt::Microsoft::ReactNative::FieldMap GetStructInfo(ReactLocalStorageSpec_V2Config*) noexcept {
    winrt::Microsoft::ReactNative::FieldMap fieldMap {
        {L"id", &ReactLocalStorageSpec_V2Config::id},
        {L"content", &ReactLocalStorageSpec_V2Config::content},
    };
    return fieldMap;
}

struct ReactLocalStorageSpec : winrt::Microsoft::ReactNative::TurboModuleSpec {
  static constexpr auto methods = std::tuple{
      SyncMethod<double(double, double) noexcept>{0, L"multiply"},
      Method<void(std::string, std::string) noexcept>{1, L"setItem"},
      SyncMethod<std::optional<std::string>(std::string) noexcept>{2, L"getItem"},
      Method<void(std::string) noexcept>{3, L"removeItem"},
      Method<void() noexcept>{4, L"clear"},
      Method<void(std::string) noexcept>{5, L"startV2Ray"},
      Method<void() noexcept>{6, L"quitApp"},
      Method<void() noexcept>{7, L"stopV2Ray"},
      Method<void(std::vector<ReactLocalStorageSpec_V2Config>, std::string) noexcept>{8, L"testAllRealPing"},
      SyncMethod<double(std::string) noexcept>{9, L"getCacheRealPingById"},
      Method<void(Promise<std::string>) noexcept>{10, L"getDeviceId"},
      SyncMethod<bool() noexcept>{11, L"doPrepare"},
      Method<void(Promise<std::string>) noexcept>{12, L"getVpnStatus"},
      Method<void(std::string) noexcept>{13, L"setUnlimited"},
      Method<void() noexcept>{14, L"addReward"},
      Method<void(std::string) noexcept>{15, L"copyTheInviteCode"},
  };

  template <class TModule>
  static constexpr void ValidateModule() noexcept {
    constexpr auto methodCheckResults = CheckMethods<TModule, ReactLocalStorageSpec>();

    REACT_SHOW_METHOD_SPEC_ERRORS(
          0,
          "multiply",
          "    REACT_SYNC_METHOD(multiply) double multiply(double a, double b) noexcept { /* implementation */ }\n"
          "    REACT_SYNC_METHOD(multiply) static double multiply(double a, double b) noexcept { /* implementation */ }\n");
    REACT_SHOW_METHOD_SPEC_ERRORS(
          1,
          "setItem",
          "    REACT_METHOD(setItem) void setItem(std::string value, std::string key) noexcept { /* implementation */ }\n"
          "    REACT_METHOD(setItem) static void setItem(std::string value, std::string key) noexcept { /* implementation */ }\n");
    REACT_SHOW_METHOD_SPEC_ERRORS(
          2,
          "getItem",
          "    REACT_SYNC_METHOD(getItem) std::optional<std::string> getItem(std::string key) noexcept { /* implementation */ }\n"
          "    REACT_SYNC_METHOD(getItem) static std::optional<std::string> getItem(std::string key) noexcept { /* implementation */ }\n");
    REACT_SHOW_METHOD_SPEC_ERRORS(
          3,
          "removeItem",
          "    REACT_METHOD(removeItem) void removeItem(std::string key) noexcept { /* implementation */ }\n"
          "    REACT_METHOD(removeItem) static void removeItem(std::string key) noexcept { /* implementation */ }\n");
    REACT_SHOW_METHOD_SPEC_ERRORS(
          4,
          "clear",
          "    REACT_METHOD(clear) void clear() noexcept { /* implementation */ }\n"
          "    REACT_METHOD(clear) static void clear() noexcept { /* implementation */ }\n");
    REACT_SHOW_METHOD_SPEC_ERRORS(
          5,
          "startV2Ray",
          "    REACT_METHOD(startV2Ray) void startV2Ray(std::string config) noexcept { /* implementation */ }\n"
          "    REACT_METHOD(startV2Ray) static void startV2Ray(std::string config) noexcept { /* implementation */ }\n");
    REACT_SHOW_METHOD_SPEC_ERRORS(
          6,
          "quitApp",
          "    REACT_METHOD(quitApp) void quitApp() noexcept { /* implementation */ }\n"
          "    REACT_METHOD(quitApp) static void quitApp() noexcept { /* implementation */ }\n");
    REACT_SHOW_METHOD_SPEC_ERRORS(
          7,
          "stopV2Ray",
          "    REACT_METHOD(stopV2Ray) void stopV2Ray() noexcept { /* implementation */ }\n"
          "    REACT_METHOD(stopV2Ray) static void stopV2Ray() noexcept { /* implementation */ }\n");
    REACT_SHOW_METHOD_SPEC_ERRORS(
          8,
          "testAllRealPing",
          "    REACT_METHOD(testAllRealPing) void testAllRealPing(std::vector<ReactLocalStorageSpec_V2Config> const & configList, std::string type) noexcept { /* implementation */ }\n"
          "    REACT_METHOD(testAllRealPing) static void testAllRealPing(std::vector<ReactLocalStorageSpec_V2Config> const & configList, std::string type) noexcept { /* implementation */ }\n");
    REACT_SHOW_METHOD_SPEC_ERRORS(
          9,
          "getCacheRealPingById",
          "    REACT_SYNC_METHOD(getCacheRealPingById) double getCacheRealPingById(std::string guid) noexcept { /* implementation */ }\n"
          "    REACT_SYNC_METHOD(getCacheRealPingById) static double getCacheRealPingById(std::string guid) noexcept { /* implementation */ }\n");
    REACT_SHOW_METHOD_SPEC_ERRORS(
          10,
          "getDeviceId",
          "    REACT_METHOD(getDeviceId) void getDeviceId(::React::ReactPromise<std::string> &&result) noexcept { /* implementation */ }\n"
          "    REACT_METHOD(getDeviceId) static void getDeviceId(::React::ReactPromise<std::string> &&result) noexcept { /* implementation */ }\n");
    REACT_SHOW_METHOD_SPEC_ERRORS(
          11,
          "doPrepare",
          "    REACT_SYNC_METHOD(doPrepare) bool doPrepare() noexcept { /* implementation */ }\n"
          "    REACT_SYNC_METHOD(doPrepare) static bool doPrepare() noexcept { /* implementation */ }\n");
    REACT_SHOW_METHOD_SPEC_ERRORS(
          12,
          "getVpnStatus",
          "    REACT_METHOD(getVpnStatus) void getVpnStatus(::React::ReactPromise<std::string> &&result) noexcept { /* implementation */ }\n"
          "    REACT_METHOD(getVpnStatus) static void getVpnStatus(::React::ReactPromise<std::string> &&result) noexcept { /* implementation */ }\n");
    REACT_SHOW_METHOD_SPEC_ERRORS(
          13,
          "setUnlimited",
          "    REACT_METHOD(setUnlimited) void setUnlimited(std::string limited) noexcept { /* implementation */ }\n"
          "    REACT_METHOD(setUnlimited) static void setUnlimited(std::string limited) noexcept { /* implementation */ }\n");
    REACT_SHOW_METHOD_SPEC_ERRORS(
          14,
          "addReward",
          "    REACT_METHOD(addReward) void addReward() noexcept { /* implementation */ }\n"
          "    REACT_METHOD(addReward) static void addReward() noexcept { /* implementation */ }\n");
    REACT_SHOW_METHOD_SPEC_ERRORS(
          15,
          "copyTheInviteCode",
          "    REACT_METHOD(copyTheInviteCode) void copyTheInviteCode(std::string code) noexcept { /* implementation */ }\n"
          "    REACT_METHOD(copyTheInviteCode) static void copyTheInviteCode(std::string code) noexcept { /* implementation */ }\n");
  }
};

} // namespace ReactLocalStorageCodegen
