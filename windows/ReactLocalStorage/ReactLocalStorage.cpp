#include "pch.h"

#include "ReactLocalStorage.h"
#include "V2rayConfigWin.h"
#include <winsqlite/winsqlite3.h>
#include <winrt/Windows.Storage.h> // Required for ApplicationData
#include <filesystem>              // Required for path operations (C++17)
#include <winrt/Windows.System.Profile.h>
#include <winrt/Windows.Security.Cryptography.h>
#include <winrt/base.h> // 提供 winrt::to_hstring
#include <string>
#include <cstdint> // 提供 int32_t
// Windows API
#include <windows.h>
#include <wincrypt.h>
#include <bcrypt.h>

#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "crypt32.lib")

// This is the new version of the function without OpenSSL dependency.
std::string getDeviceIdForXUDPBaseKey() {
  // 1. Generate 32 bytes of cryptographically secure random data using Windows CNG API
  unsigned char randomData[32];
  NTSTATUS status = BCryptGenRandom(
    NULL,                   // Use the default RNG provider
    randomData,             // Buffer to fill
    sizeof(randomData),     // Size of the buffer
    BCRYPT_USE_SYSTEM_PREFERRED_RNG); // Flags

  if (!BCRYPT_SUCCESS(status)) {
    throw std::runtime_error("Failed to generate random bytes using BCryptGenRandom");
  }

  // 2. Base64 encode the random data using Windows CryptoAPI
  DWORD base64StringSize = 0;
  // First, call to get the required buffer size
  if (!CryptBinaryToStringA(randomData, sizeof(randomData), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, &base64StringSize)) {
    throw std::runtime_error("Failed to get Base64 string size");
  }

  // Allocate a buffer and call again to perform the encoding
  std::vector<char> base64Buffer(base64StringSize);
  if (!CryptBinaryToStringA(randomData, sizeof(randomData), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, base64Buffer.data(), &base64StringSize)) {
    throw std::runtime_error("Failed to perform Base64 encoding");
  }

  // Create a std::string from the result (excluding the null terminator)
  std::string base64String(base64Buffer.data(), base64StringSize - 1);

  // 3. The rest of the logic is standard string manipulation and remains the same

  // Remove padding characters ('=')
  base64String.erase(std::remove(base64String.begin(), base64String.end(), '='), base64String.end());

  // Replace URL-unsafe characters
  std::replace(base64String.begin(), base64String.end(), '+', '-');
  std::replace(base64String.begin(), base64String.end(), '/', '_');

  return base64String;
}
using namespace std::string_literals;
using namespace winrt::Microsoft::ReactNative;
using namespace std::chrono_literals;
namespace winrt::ReactLocalStorage
{

// Helper function to convert hstring to std::string
std::string to_string(winrt::hstring const& hstr)
{
    if (hstr.empty())
    {
        return {};
    }
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, hstr.c_str(), (int)hstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, hstr.c_str(), (int)hstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}
std::string ReactLocalStorage::GetDbPath() noexcept
{
    try
    {
        winrt::Windows::Storage::StorageFolder localFolder = winrt::Windows::Storage::ApplicationData::Current().LocalFolder();
        std::filesystem::path dbPath(to_string(localFolder.Path()));
        dbPath /= "react_local_storage.db";
        return dbPath.string();
    }
    catch (winrt::hresult_error const& ex)
    {
        // Log error or handle appropriately
        OutputDebugStringA(("Failed to get DB path: " + to_string(ex.message()) + "\n").c_str());
        return {}; // Return empty path on error
    }
}

void ReactLocalStorage::EnsureDbOpen() noexcept
{
    if (m_db)
    {
        return; // Already open
    }

    std::string dbPath = GetDbPath();
    if (dbPath.empty())
    {
        OutputDebugStringA("DB path is empty, cannot open database.\n");
        return;
    }

    int rc = sqlite3_open_v2(dbPath.c_str(), &m_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (rc != SQLITE_OK)
    {
        OutputDebugStringA(("Failed to open database: " + std::string(sqlite3_errmsg(m_db)) + "\n").c_str());
        sqlite3_close(m_db); // sqlite3_close can be called on a null pointer or an unopened db
        m_db = nullptr;
        return;
    }

    // Create table if it doesn't exist
    const char* createTableSql = "CREATE TABLE IF NOT EXISTS key_value_store (item_key TEXT PRIMARY KEY NOT NULL, item_value TEXT);";
    char* errMsg = nullptr;
    rc = sqlite3_exec(m_db, createTableSql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        OutputDebugStringA(("Failed to create table: " + std::string(errMsg) + "\n").c_str());
        sqlite3_free(errMsg);
        CloseDb(); // Close DB if table creation fails
    }
}

void ReactLocalStorage::CloseDb() noexcept
{
    if (m_db)
    {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

ReactLocalStorage::~ReactLocalStorage()
{
    CloseDb();
}
// See https://microsoft.github.io/react-native-windows/docs/native-modules for details on writing native modules

void ReactLocalStorage::Initialize(React::ReactContext const &reactContext) noexcept {
  m_context = reactContext;
  EnsureDbOpen();
}

double ReactLocalStorage::multiply(double a, double b) noexcept {
  return a * b;
}

void ReactLocalStorage::setItem(std::string value, std::string key) noexcept
{
    EnsureDbOpen();
    if (!m_db) return;

    const char* sql = "INSERT OR REPLACE INTO key_value_store (item_key, item_value) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK)
    {
        OutputDebugStringA(("setItem: Failed to prepare statement: " + std::string(sqlite3_errmsg(m_db)) + "\n").c_str());
        return;
    }

    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, value.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        OutputDebugStringA(("setItem: Failed to execute statement: " + std::string(sqlite3_errmsg(m_db)) + "\n").c_str());
    }
    sqlite3_finalize(stmt);
}

std::optional<std::string> ReactLocalStorage::getItem(std::string key) noexcept
{
    EnsureDbOpen();
    if (!m_db) return std::nullopt;

    const char* sql = "SELECT item_value FROM key_value_store WHERE item_key = ?;";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK)
    {
        OutputDebugStringA(("getItem: Failed to prepare statement: " + std::string(sqlite3_errmsg(m_db)) + "\n").c_str());
        return std::nullopt;
    }

    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);

    std::optional<std::string> result = std::nullopt;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        const unsigned char* text = sqlite3_column_text(stmt, 0);
        if (text)
        {
            result = reinterpret_cast<const char*>(text);
        }
    }
    else if (rc != SQLITE_DONE) // SQLITE_DONE means no row found, which is fine
    {
        OutputDebugStringA(("getItem: Failed to execute statement: " + std::string(sqlite3_errmsg(m_db)) + "\n").c_str());
    }
    sqlite3_finalize(stmt);
    return result;
}

void ReactLocalStorage::removeItem(std::string key) noexcept
{
    EnsureDbOpen();
    if (!m_db) return;

    const char* sql = "DELETE FROM key_value_store WHERE item_key = ?;";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK)
    {
        OutputDebugStringA(("removeItem: Failed to prepare statement: " + std::string(sqlite3_errmsg(m_db)) + "\n").c_str());
        return;
    }

    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        OutputDebugStringA(("removeItem: Failed to execute statement: " + std::string(sqlite3_errmsg(m_db)) + "\n").c_str());
    }
    sqlite3_finalize(stmt);
}

void ReactLocalStorage::clear() noexcept
{
    EnsureDbOpen();
    if (!m_db) return;

    const char* sql = "DELETE FROM key_value_store;";
    char* errMsg = nullptr;
    int rc = sqlite3_exec(m_db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        OutputDebugStringA(("clear: Failed to execute statement: " + std::string(errMsg) + "\n").c_str());
        sqlite3_free(errMsg);
    }
}

void ReactLocalStorage::SendLogToJS(std::string const& message) noexcept {
     if (!m_context) {
        #ifdef _DEBUG
            OutputDebugString(L"ReactLocalStorage::SendLogToJS: m_context is null\n");
        #endif
        return;
      }
       // FIX: Explicitly cast the lambda to the expected delegate type 'JSValueArgWriter'.
    // This helps the compiler resolve the correct overload of DispatchEvent.
    m_context.EmitJSEvent(
        L"RCTDeviceEventEmitter",
        L"NativeLog",
        JSValueArgWriter(
            [&message](IJSValueWriter const& writer) noexcept {
                writer.WriteObjectBegin();
                writer.WritePropertyName(L"message");
                writer.WriteString(winrt::to_hstring(message));
                writer.WriteObjectEnd();
            }
        )
    );
}
// =================================================================
        // 步骤 1: 定义 V2Ray 所需的回调函数
        // 这些函数必须是静态的，因为它们将被传递给一个 C 库。
        // =================================================================

static int OnSetup(int handle, const char* conf) {
  //Logger::WriteMessage((L"Callback OnSetup called for handle: " + std::to_wstring(handle)).c_str());
  return 0; // 返回 0 表示成功
}

static int OnPrepare(int handle) {
  //Logger::WriteMessage((L"Callback OnPrepare called for handle: " + std::to_wstring(handle)).c_str());
  return 0;
}

static int OnShutdown(int handle) {
  //Logger::WriteMessage((L"Callback OnShutdown called for handle: " + std::to_wstring(handle)).c_str());
  return 0;
}

static bool OnProtect(int handle, int fd) {
  //Logger::WriteMessage((L"Callback OnProtect called for handle: " + std::to_wstring(handle) + L", fd: " + std::to_wstring(fd)).c_str());
  return false; // 在非 VPN 模式下，总是返回 false
}

static int OnEmitStatus(int handle, int status, const char* msg) {
  std::wstring wide_msg;
  if (msg) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, msg, -1, NULL, 0);
    wide_msg.resize(size_needed);
    MultiByteToWideChar(CP_UTF8, 0, msg, -1, &wide_msg[0], size_needed);
  }
  //Logger::WriteMessage((L"Callback OnEmitStatus for handle " + std::to_wstring(handle) + L": " + wide_msg.c_str()).c_str());
  return 0;
}

// 这是将在后台线程中执行的函数
void ReactLocalStorage::V2RayThreadWorker(std::string config)
{
    // 使用 try-catch 捕获所有潜在的异常，防止后台线程崩溃
    try
    {
        //SendLogToJS("V2Ray thread started.");
        std::optional<V2rayConfigWin::ServerConfig> result = V2rayConfigWin::AngConfigManager::importConfig(config);
        if (!result.has_value()) {
            //SendLogToJS("Error: Failed to parse V2Ray config.");
            return;
        }

        std::optional<std::string> configStrOpt = V2rayConfigWin::V2rayConfigGenerator::generate(result.value(), V2rayConfigWin::V2rayGeneratorSettings{});
        if (!configStrOpt.has_value()) {
            //SendLogToJS("Error: Failed to generate V2Ray JSON config string.");
            return;
        }
        
        // InitV2Env 只需要调用一次
        // 注意：在实际应用中，这个路径应该是动态获取或打包到应用内的
        std::string baseKey = getDeviceIdForXUDPBaseKey();
        m_v2rayManager.InitV2Env("F:\\dev\\apps\\react-local-storage\\windows\\ReactLocalStorage\\libv2ray", baseKey);
        //SendLogToJS("InitV2Env completed.");

        m_v2rayManager.SetCallbacks(&OnSetup, &OnPrepare, &OnShutdown, &OnProtect, &OnEmitStatus);
        //SendLogToJS("SetCallbacks completed.");

        int handle = m_v2rayManager.CreateV2RayPoint(true);
        if (handle < 0) {
            //SendLogToJS("Error: CreateV2RayPoint failed.");
            return;
        }

        // 启动成功后，更新状态
        {
            std::lock_guard<std::mutex> lock(m_v2rayMutex);
            m_v2rayHandle = handle;
            m_isV2RayRunning = true;
        }
        //SendLogToJS("V2Ray instance created with handle: " + std::to_string(handle));
        std::string domain(result.value().outboundBean.value().settings.value().vnext.value().at(0).address + ":" + std::to_string(result.value().outboundBean.value().settings.value().vnext.value().at(0).port));
    
        // 这是阻塞调用，它将使这个后台线程持续运行
        std::string startResult = m_v2rayManager.StartV2RayPoint(handle, false, domain, configStrOpt.value());
        
        // 当 StartV2RayPoint 返回时，意味着 V2Ray 已经停止
        //SendLogToJS("V2Ray has stopped. Reason: " + (startResult.empty() ? "Normal shutdown" : startResult));

    }
    catch (const std::exception& e)
    {
        SendLogToJS("Exception in V2Ray thread: " + std::string(e.what()));
    }
    catch (...)
    {
        SendLogToJS("Unknown exception in V2Ray thread.");
    }

    // 线程结束前，重置状态
    {
        std::lock_guard<std::mutex> lock(m_v2rayMutex);
        m_isV2RayRunning = false;
        m_v2rayHandle = -1;
    }
}
void ReactLocalStorage::startV2Ray(std::string config) noexcept
{
    // TODO: Implement startV2Ray
    // std::optional<V2rayConfigWin::ServerConfig> result = V2rayConfigWin::AngConfigManager::importConfig(config);
    // std::optional<std::string> baseKey = getDeviceIdForXUDPBaseKey();
    // std::cout << "Base Key: " << (baseKey.has_value() ? baseKey.value() : "No value") << std::endl;
    // std::optional<std::string> configStrOpt = V2rayConfigWin::V2rayConfigGenerator::generate(result.value(), V2rayConfigWin::V2rayGeneratorSettings{});
    // std::cout << "configStrOpt: " << (configStrOpt.has_value() ? configStrOpt.value() : "No value") << std::endl;
    // V2rayManager v2rayManager;
    // v2rayManager.InitV2Env("F:\\dev\\apps\\react-local-storage\\windows\\ReactLocalStorage\\libv2ray", baseKey.value());
    // std::cout << "InitV2Env completed. " << std::endl;
    // v2rayManager.SetCallbacks(&OnSetup, &OnPrepare, &OnShutdown, &OnProtect, &OnEmitStatus);
    // std::cout << "SetCallbacks completed. " << std::endl;
    // int handle = v2rayManager.CreateV2RayPoint(true);
    // std::cout << "CreateV2RayPoint returned handle: " << handle << std::endl;
    // std::string domain(result.value().outboundBean.value().settings.value().vnext.value().at(0).address + ":" + std::to_string(result.value().outboundBean.value().settings.value().vnext.value().at(0).port));
    // // (可选) 打印出来确认结果
    // std::cout << "Constructed domain: " << domain << std::endl;
    // std::string startResult = v2rayManager.StartV2RayPoint(handle, false, domain, configStrOpt.value());
    // std::cout << "StartV2RayPoint command sent successfully: " << startResult << std::endl;
        // 使用互斥锁保护，防止同时多次调用
    std::lock_guard<std::mutex> lock(m_v2rayMutex);

    if (m_isV2RayRunning) {
        SendLogToJS("V2Ray is already running.");
        return;
    }

    // 如果上一个线程还存在（虽然不太可能，但作为安全措施），先 join
    if (m_v2rayThread.joinable()) {
        m_v2rayThread.join();
    }

    // 创建并启动一个新的后台线程
    // this 指针作为第二个参数，是因为 V2RayThreadWorker 是一个成员函数
    m_v2rayThread = std::thread(&ReactLocalStorage::V2RayThreadWorker, this, config);
    
    // 分离线程，让它在后台自由运行，我们不再直接管理它
    // UI 线程可以立即返回，不会被阻塞
    m_v2rayThread.detach(); 
}

void ReactLocalStorage::quitApp() noexcept
{
    // TODO: Implement quitApp
    OutputDebugStringA("quitApp called\n");
}

void ReactLocalStorage::stopV2Ray() noexcept
{
    std::lock_guard<std::mutex> lock(m_v2rayMutex);

    if (!m_isV2RayRunning || m_v2rayHandle < 0) {
        SendLogToJS("V2Ray is not running or handle is invalid.");
        return;
    }

    SendLogToJS("Stopping V2Ray with handle: " + std::to_string(m_v2rayHandle));
    
    // 这个调用会使 V2Ray 核心停止，从而让后台线程中的 StartV2RayPoint 返回
    m_v2rayManager.StopV2RayPoint(m_v2rayHandle);
    m_v2rayManager.FreeV2RayPoint(m_v2rayHandle);
}

void ReactLocalStorage::testAllRealPing(
    std::vector<ReactLocalStorageCodegen::ReactLocalStorageSpec_V2Config> const & configList,
     std::string type) noexcept
{
    // TODO: Implement testAllRealPing
    OutputDebugStringA("testAllRealPing called\n");
}

double ReactLocalStorage::getCacheRealPingById(std::string guid) noexcept
{
    // TODO: Implement getCacheRealPingById
    OutputDebugStringA("getCacheRealPingById called\n");
    return -1.0;
}

void ReactLocalStorage::getDeviceId(winrt::Microsoft::ReactNative::ReactPromise<std::string> &&result) noexcept
{
    try
    {
        auto systemIdInfo = winrt::Windows::System::Profile::SystemIdentification::GetSystemIdForPublisher();
        
        // Check if a valid ID was obtained.
        if (systemIdInfo.Source() == winrt::Windows::System::Profile::SystemIdentificationSource::None)
        {
             result.Reject(winrt::Microsoft::ReactNative::ReactError{ 
                 "E_NO_DEVICE_ID", 
                 "Could not retrieve a unique device identifier." });
             return;
        }

        auto idBuffer = systemIdInfo.Id();
        // Encode the buffer to a hex string to get a readable and consistent representation.
        winrt::hstring deviceIdAsHexString = winrt::Windows::Security::Cryptography::CryptographicBuffer::EncodeToHexString(idBuffer);
        
        result.Resolve(to_string(deviceIdAsHexString));
    }
    catch (winrt::hresult_error const& ex)
    {
        result.Reject(winrt::Microsoft::ReactNative::ReactError{ "E_DEVICE_ID_ERROR",to_string(ex.message())});
    }
}

bool ReactLocalStorage::doPrepare() noexcept
{
    // TODO: Implement doPrepare
    OutputDebugStringA("doPrepare called\n");
    return true;
}
 
void ReactLocalStorage::getVpnStatus(winrt::Microsoft::ReactNative::ReactPromise<std::string> &&result) noexcept
{
    // TODO: Implement getVpnStatus
    OutputDebugStringA("getVpnStatus called\n");
    //result.Resolve("Disconnected");
}

void ReactLocalStorage::setUnlimited(std::string limited) noexcept
{
    // TODO: Implement setUnlimited
    OutputDebugStringA("setUnlimited called\n");
}

void ReactLocalStorage::addReward() noexcept
{
    // TODO: Implement addReward
    OutputDebugStringA("addReward called\n");
}

void ReactLocalStorage::copyTheInviteCode(std::string code) noexcept
{
    // TODO: Implement copyTheInviteCode
    OutputDebugStringA("copyTheInviteCode called\n");
}

// FIX: Add implementation for the required NativeEventEmitter methods.
// These can be empty for now. Their existence is what's important.
void ReactLocalStorage::addListener(std::string const& /*eventName*/) noexcept
{
    // This method is required by NativeEventEmitter.
    // You can add logic here if you need to track listeners, but it's often not necessary.
}

void ReactLocalStorage::removeListeners(double /*count*/) noexcept
{
    // This method is required by NativeEventEmitter.
}
} // namespace winrt::ReactLocalStorage
