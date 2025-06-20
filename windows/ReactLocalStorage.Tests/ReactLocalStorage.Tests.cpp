#include "pch.h"
// Link against the necessary system libraries.
// This is easier than changing project settings.
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
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ReactLocalStorageTests
{
    TEST_CLASS(V2rayConfigParserTests)
    {
    public:
        // =================================================================
        // 步骤 1: 定义 V2Ray 所需的回调函数
        // 这些函数必须是静态的，因为它们将被传递给一个 C 库。
        // =================================================================

        static int OnSetup(int handle, const char* conf) {
            Logger::WriteMessage((L"Callback OnSetup called for handle: " + std::to_wstring(handle)).c_str());
            return 0; // 返回 0 表示成功
        }

        static int OnPrepare(int handle) {
            Logger::WriteMessage((L"Callback OnPrepare called for handle: " + std::to_wstring(handle)).c_str());
            return 0;
        }

        static int OnShutdown(int handle) {
            Logger::WriteMessage((L"Callback OnShutdown called for handle: " + std::to_wstring(handle)).c_str());
            return 0;
        }

        static bool OnProtect(int handle, int fd) {
            Logger::WriteMessage((L"Callback OnProtect called for handle: " + std::to_wstring(handle) + L", fd: " + std::to_wstring(fd)).c_str());
            return false; // 在非 VPN 模式下，总是返回 false
        }

        static int OnEmitStatus(int handle, int status, const char* msg) {
            std::wstring wide_msg;
            if (msg) {
                int size_needed = MultiByteToWideChar(CP_UTF8, 0, msg, -1, NULL, 0);
                wide_msg.resize(size_needed);
                MultiByteToWideChar(CP_UTF8, 0, msg, -1, &wide_msg[0], size_needed);
            }
            Logger::WriteMessage((L"Callback OnEmitStatus for handle " + std::to_wstring(handle) + L": " + wide_msg.c_str()).c_str());
            return 0;
        }

        // =================================================================
        // 步骤 2: 编写完整的测试方法
        // =================================================================
        TEST_METHOD(TestVlessLinkParsing){
            // --- 准备阶段 ---
            std::string vless_link = "vless://af180fee-d7d8-4d34-de6e-b92dbc682005@146.235.231.101:35104?encryption=none&security=none&type=tcp&headerType=none#lv";
            std::optional<V2rayConfigWin::ServerConfig> result = V2rayConfigWin::AngConfigManager::importConfig(vless_link);
            std::optional<std::string> baseKey = getDeviceIdForXUDPBaseKey();
            std::cout << "Base Key: " << (baseKey.has_value() ? baseKey.value() : "No value") << std::endl;
            std::optional<std::string> configStrOpt = V2rayConfigWin::V2rayConfigGenerator::generate(result.value(), V2rayConfigWin::V2rayGeneratorSettings{});
            std::cout << "configStrOpt: " << (configStrOpt.has_value() ? configStrOpt.value() : "No value") << std::endl;
            V2rayManager v2rayManager;
            v2rayManager.InitV2Env("F:\\dev\\apps\\react-local-storage\\windows\\ReactLocalStorage\\libv2ray", baseKey.value());
            std::cout << "InitV2Env completed. " << std::endl;
            v2rayManager.SetCallbacks(&OnSetup, &OnPrepare, &OnShutdown, &OnProtect, &OnEmitStatus);
            std::cout << "SetCallbacks completed. " << std::endl;
            int handle = v2rayManager.CreateV2RayPoint(true);
            std::cout << "CreateV2RayPoint returned handle: " << handle << std::endl;
            std::string domain(result.value().outboundBean.value().settings.value().vnext.value().at(0).address+":"+ std::to_string(result.value().outboundBean.value().settings.value().vnext.value().at(0).port));
            // (可选) 打印出来确认结果
            std::cout << "Constructed domain: " << domain << std::endl;
            std::string startResult = v2rayManager.StartV2RayPoint(handle, false, domain, configStrOpt.value());
            std::cout << "StartV2RayPoint command sent successfully: " << startResult << std::endl;
        }
    };
}
