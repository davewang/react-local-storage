
/*
 * This file is auto-generated from a NativeModule spec file in js.
 *
 * This is a C++ Spec class that should be used with MakeTurboModuleProvider to register native modules
 * in a way that also verifies at compile time that the native module matches the interface required
 * by the TurboModule JS spec.
 */
#pragma once
// clang-format off


#include <NativeModules.h>
#include <tuple>

namespace ReactLocalStorageCodegen {

struct ReactLocalStorageSpec : winrt::Microsoft::ReactNative::TurboModuleSpec {
  static constexpr auto methods = std::tuple{
      SyncMethod<double(double, double) noexcept>{0, L"multiply"},
      Method<void(std::string, std::string) noexcept>{1, L"setItem"},
      SyncMethod<std::optional<std::string>(std::string) noexcept>{2, L"getItem"},
      Method<void(std::string) noexcept>{3, L"removeItem"},
      Method<void() noexcept>{4, L"clear"},
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
  }
};

} // namespace ReactLocalStorageCodegen
