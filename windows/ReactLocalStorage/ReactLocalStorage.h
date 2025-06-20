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
#include "V2rayManager.h"
#include <thread>          // 包含线程库
#include <mutex>           // 包含互斥锁库
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

   REACT_METHOD(startV2Ray)
  void startV2Ray(std::string config) noexcept;

  REACT_METHOD(quitApp)
  void quitApp() noexcept;

  REACT_METHOD(stopV2Ray)
  void stopV2Ray() noexcept;

  REACT_METHOD(testAllRealPing)
  void testAllRealPing(std::vector<ReactLocalStorageCodegen::ReactLocalStorageSpec_V2Config> const & configList, std::string type) noexcept;

  REACT_SYNC_METHOD(getCacheRealPingById)
  double getCacheRealPingById(std::string guid) noexcept;

  REACT_METHOD(getDeviceId)
  void getDeviceId(winrt::Microsoft::ReactNative::ReactPromise<std::string> &&result) noexcept;

  REACT_SYNC_METHOD(doPrepare)
  bool doPrepare() noexcept;

  REACT_METHOD(getVpnStatus)
  void getVpnStatus(winrt::Microsoft::ReactNative::ReactPromise<std::string> &&result) noexcept;
 
  REACT_METHOD(setUnlimited)
  void setUnlimited(std::string limited) noexcept;

  REACT_METHOD(addReward)
  void addReward() noexcept;

  REACT_METHOD(copyTheInviteCode)
  void copyTheInviteCode(std::string code) noexcept;

  // FIX: Add required methods for NativeEventEmitter
  REACT_METHOD(addListener)
  void addListener(std::string const& eventName) noexcept;

  REACT_METHOD(removeListeners)
  void removeListeners(double count) noexcept;
private:
  void SendLogToJS(std::string const& message) noexcept;
  React::ReactContext m_context;
  sqlite3* m_db{nullptr}; // SQLite database connection
  // --- V2Ray 后台任务管理 ---
  V2rayManager m_v2rayManager;
  std::thread m_v2rayThread;
  std::mutex m_v2rayMutex;
  int m_v2rayHandle{-1}; // -1 表示无效句柄
  bool m_isV2RayRunning{false};

  // 后台线程的工作函数
  void V2RayThreadWorker(std::string config);

  std::string GetDbPath() noexcept;
  void EnsureDbOpen() noexcept;
  void CloseDb() noexcept;
};

} // namespace winrt::ReactLocalStorage
