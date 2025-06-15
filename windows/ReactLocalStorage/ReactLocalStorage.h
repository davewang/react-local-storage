#pragma once

#include "pch.h"
#include "resource.h"

#if __has_include("codegen/NativeReactLocalStorageDataTypes.g.h")
  #include "codegen/NativeReactLocalStorageDataTypes.g.h"
#endif
#include "codegen/NativeReactLocalStorageSpec.g.h"

#include "NativeModules.h"
#include <optional> // Required for std::optional
#include <string>   // Required for std::string

// Forward declare sqlite3
struct sqlite3;
namespace winrt::ReactLocalStorage
{

REACT_MODULE(ReactLocalStorage)
struct ReactLocalStorage
{

  using ModuleSpec = ReactLocalStorageCodegen::ReactLocalStorageSpec;

  ReactLocalStorage() = default;
  ~ReactLocalStorage();
  
  REACT_INIT(Initialize)
  void Initialize(React::ReactContext const &reactContext) noexcept;

  REACT_SYNC_METHOD(multiply)
  double multiply(double a, double b) noexcept;

  REACT_METHOD(setItem)
  void setItem(std::string value, std::string key) noexcept;

  REACT_SYNC_METHOD(getItem)
  std::optional<std::string> getItem(std::string key) noexcept;

  REACT_METHOD(removeItem)
  void removeItem(std::string key) noexcept;

  REACT_METHOD(clear)
  void clear() noexcept;

private:
  React::ReactContext m_context;
  sqlite3* m_db{nullptr}; // SQLite database connection

  std::string GetDbPath() noexcept;
  void EnsureDbOpen() noexcept;
  void CloseDb() noexcept;
};

} // namespace winrt::ReactLocalStorage
