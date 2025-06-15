#include "pch.h"

#include "ReactLocalStorage.h"
#include <winsqlite/winsqlite3.h>
#include <winrt/Windows.Storage.h> // Required for ApplicationData
#include <filesystem>              // Required for path operations (C++17)
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


} // namespace winrt::ReactLocalStorage