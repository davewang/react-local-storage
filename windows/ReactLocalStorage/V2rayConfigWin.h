#pragma once

#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <map>
#include <any>
#include <stdexcept>
#include <regex>
#include <sstream>
#include <iostream>
#include <nlohmann/json.hpp>
namespace nlohmann {
    template <typename T>
    struct adl_serializer<std::optional<T>> {
        static void to_json(json& j, const std::optional<T>& opt) {
            if (opt.has_value()) {
                j = *opt;
            } else {
                j = nullptr;
            }
        }

        static void from_json(const json& j, std::optional<T>& opt) {
            if (j.is_null()) {
                opt = std::nullopt;
            } else {
                opt = j.get<T>();
            }
        }
    };
}
// =================================================================================
// Main Namespace
// =================================================================================
namespace V2rayConfigWin
{
    // =================================================================================
    // Utility Namespace
    // =================================================================================
    namespace Utils
    {
        // WARNING: Basic Base64 decoder. REPLACE with a robust library for production.
        inline std::string decode_base64(const std::string& in) {
            std::string out;
            std::vector<int> T(256, -1);
            for (int i = 0; i < 64; i++) T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;
            int val = 0, valb = -8;
            for (char c : in) {
                if (c == '=') break;
                if (T[c] == -1) continue;
                val = (val << 6) + T[c];
                valb += 6;
                if (valb >= 0) {
                    out.push_back(char((val >> valb) & 0xFF));
                    valb -= 8;
                }
            }
            return out;
        }

        // WARNING: Basic URL decoder. REPLACE with a robust library for production.
        inline std::string url_decode(const std::string& in) {
            std::string out;
            out.reserve(in.length());
            for (std::size_t i = 0; i < in.length(); ++i) {
                if (in[i] == '%') {
                    if (i + 3 <= in.length()) {
                        int value = 0;
                        std::istringstream is(in.substr(i + 1, 2));
                        if (is >> std::hex >> value) {
                            out += static_cast<char>(value);
                            i += 2;
                        } else { out += '%'; }
                    } else { out += '%'; }
                } else if (in[i] == '+') {
                    out += ' ';
                } else { out += in[i]; }
            }
            return out;
        }

        inline int parse_int(const std::string& s, int default_val = 0) {
            try {
                return std::stoi(s);
            } catch (...) {
                return default_val;
            }
        }

        // A simple helper to split strings
        inline std::vector<std::string> split(const std::string& s, char delimiter) {
            std::vector<std::string> tokens;
            std::string token;
            std::istringstream tokenStream(s);
            while (std::getline(tokenStream, token, delimiter)) {
                tokens.push_back(token);
            }
            return tokens;
        }

        // NEW: Helper to parse URL query parameters
        inline std::map<std::string, std::string> parse_query_params(const std::string& query) {
            std::map<std::string, std::string> params;
            auto pairs = split(query, '&');
            for (const auto& pair : pairs) {
                auto kv = split(pair, '=');
                if (kv.size() == 2) {
                    params[kv[0]] = url_decode(kv[1]);
                }
            }
            return params;
        }

        // NEW: Utility functions translated from Kotlin
        inline bool is_ip_address(const std::string& str) {
            // This is a simplified check. For production, a more robust regex might be needed.
            std::regex ip_pattern(R"(^(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})|(\[([0-9a-fA-F:]+)\])$)");
            return std::regex_match(str, ip_pattern);
        }
        inline bool is_pure_ip_address(const std::string& str) {
            std::regex ip_pattern(R"(^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$)");
            return std::regex_match(str, ip_pattern);
        }
        inline std::vector<std::string> get_remote_dns_servers() {
            // In a real app, this would come from user settings.
            return { "1.1.1.1", "8.8.8.8" };
        }
        inline std::vector<std::string> get_domestic_dns_servers() {
            return { "223.5.5.5", "119.29.29.29" };
        }
    } // namespace Utils

    // =================================================================================
    // Data Transfer Objects (DTOs) for Link Parsing
    // =================================================================================

    enum class EConfigType { VMESS = 1, CUSTOM = 2, SHADOWSOCKS = 3, SOCKS = 4, VLESS = 5, TROJAN = 6, WIREGUARD = 7 };
    inline std::string get_protocol_scheme(EConfigType type) {
        switch (type) {
            case EConfigType::VMESS: return "vmess://";
            case EConfigType::SHADOWSOCKS: return "ss://";
            case EConfigType::SOCKS: return "socks://";
            case EConfigType::VLESS: return "vless://";
            case EConfigType::TROJAN: return "trojan://";
            case EConfigType::WIREGUARD: return "wireguard://";
            default: return "";
        }
    }

    struct VmessQRCode {
        std::string v = "2", ps = "", add = "", port = "", id = "", aid = "0", scy = "auto", net = "", type = "", host = "", path = "", tls = "", sni = "", alpn = "", fp = "";
    };

    // JSON mapping for VmessQRCode
    inline void from_json(const nlohmann::json& j, VmessQRCode& p) {
        p.ps = j.value("ps", "");
        p.add = j.value("add", "");
        p.port = j.value("port", "");
        p.id = j.value("id", "");
        p.aid = j.value("aid", "0");
        p.scy = j.value("scy", "auto");
        p.net = j.value("net", "");
        p.type = j.value("type", "");
        p.host = j.value("host", "");
        p.path = j.value("path", "");
        p.tls = j.value("tls", "");
        p.sni = j.value("sni", "");
        p.alpn = j.value("alpn", "");
        p.fp = j.value("fp", "");
    }

    namespace V2rayConfig {
        const std::string DEFAULT_SECURITY = "auto";
        const std::string TLS = "tls";

        struct TlsSettings {
            std::optional<std::string> serverName;
            std::optional<bool> allowInsecure;
            std::optional<std::string> fingerprint;
            std::optional<std::vector<std::string>> alpn;
            NLOHMANN_DEFINE_TYPE_INTRUSIVE(TlsSettings, serverName, allowInsecure, fingerprint, alpn);
        };

        struct StreamSettings {
            std::optional<std::string> network;
            std::optional<std::string> security;
            std::optional<TlsSettings> tlsSettings;
            
            // Equivalent of populateTlsSettings
            inline void populateTlsSettings(const std::string& security, bool allowInsecure, const std::string& sni, const std::string& fingerprint, const std::string& alpn,
                std::optional<std::string>, std::optional<std::string>, std::optional<std::string>)
            {
                if (security == TLS) {
                    this->security = TLS;
                    TlsSettings settings;
                    settings.allowInsecure = allowInsecure;
                    if (!sni.empty()) settings.serverName = sni;
                    if (!fingerprint.empty()) settings.fingerprint = fingerprint;
                    if (!alpn.empty()) settings.alpn = Utils::split(alpn, ',');
                    this->tlsSettings = settings;
                }
            }
            // Simplified equivalent of populateTransportSettings
            inline std::string populateTransportSettings(const std::string& network, const std::optional<std::string>&,
                const std::optional<std::string>& host, const std::optional<std::string>&,
                const std::optional<std::string>&, const std::optional<std::string>&,
                const std::optional<std::string>&, const std::optional<std::string>&,
                const std::optional<std::string>&)
            {
                this->network = network;
                // This is where you would populate wsSettings, tcpSettings, etc.
                // For now, just returning the host as a potential SNI.
                return host.value_or("");
            }
            NLOHMANN_DEFINE_TYPE_INTRUSIVE(StreamSettings, network, security, tlsSettings);
        };

        struct VnextUserBean {
            std::string id;
            int alterId = 0;
            std::string security = DEFAULT_SECURITY;
            std::string encryption = "none";
            std::string flow = "";
            NLOHMANN_DEFINE_TYPE_INTRUSIVE(VnextUserBean, id, alterId, security, encryption, flow);
        };

        struct VnextServerBean {
            std::string address;
            int port = 0;
            std::vector<VnextUserBean> users;
            NLOHMANN_DEFINE_TYPE_INTRUSIVE(VnextServerBean, address, port, users);
        };

        struct SocksUserBean {
            std::string user;
            std::string pass;
            NLOHMANN_DEFINE_TYPE_INTRUSIVE(SocksUserBean, user, pass);
        };

        struct ServerObjectBean {
            std::string address;
            int port = 0;
            std::string method;
            std::string password;
            std::optional<std::vector<SocksUserBean>> users;
            std::string flow;
            NLOHMANN_DEFINE_TYPE_INTRUSIVE(ServerObjectBean, address, port, method, password, users, flow);
        };

        struct OutboundSettings {
            std::optional<std::vector<VnextServerBean>> vnext;
            std::optional<std::vector<ServerObjectBean>> servers;
            NLOHMANN_DEFINE_TYPE_INTRUSIVE(OutboundSettings, vnext, servers);
        };

        struct OutboundBean {
            std::string protocol;
            std::optional<OutboundSettings> settings;
            std::optional<StreamSettings> streamSettings;
            std::optional<std::string> tag;
            NLOHMANN_DEFINE_TYPE_INTRUSIVE(OutboundBean, protocol, settings, streamSettings, tag);
        };
    }

    struct ServerConfig {
        int configVersion = 3;
        EConfigType configType;
        std::string subscriptionId = "";
        long long addedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        std::string remarks = "";
        std::optional<V2rayConfig::OutboundBean> outboundBean;

        // Factory method, equivalent to ServerConfig.create(EConfigType)
        static inline ServerConfig create(EConfigType type) {
            ServerConfig config{ .configType = type };
            V2rayConfig::OutboundBean outbound;
            outbound.streamSettings = V2rayConfig::StreamSettings{};
            V2rayConfig::OutboundSettings settings;

            switch (type) {
                case EConfigType::VMESS:
                    outbound.protocol = "vmess";
                    settings.vnext = std::vector<V2rayConfig::VnextServerBean>{ { .users = std::vector<V2rayConfig::VnextUserBean>{ {} } } };
                    break;
                case EConfigType::VLESS:
                    outbound.protocol = "vless";
                    settings.vnext = std::vector<V2rayConfig::VnextServerBean>{ { .users = std::vector<V2rayConfig::VnextUserBean>{ {} } } };
                    break;
                case EConfigType::SHADOWSOCKS:
                    outbound.protocol = "shadowsocks";
                    settings.servers = std::vector<V2rayConfig::ServerObjectBean>{ {} };
                    break;
                case EConfigType::SOCKS:
                    outbound.protocol = "socks";
                    settings.servers = std::vector<V2rayConfig::ServerObjectBean>{ {} };
                    break;
                case EConfigType::TROJAN:
                    outbound.protocol = "trojan";
                    settings.servers = std::vector<V2rayConfig::ServerObjectBean>{ {} };
                    break;
                default:
                    break;
            }
            outbound.settings = settings;
            config.outboundBean = outbound;
            return config;
        }
    };

    // =================================================================================
    // AngConfigManager Logic (Link Parser)
    // =================================================================================
    namespace AngConfigManager {
        // Forward declarations for internal helpers
        inline bool tryResolveVmess4Kitsunebi(const std::string& server, ServerConfig& config);

        inline std::optional<ServerConfig> importConfig(const std::string& str) {
            if (str.empty()) {
                return std::nullopt;
            }

            try {
                ServerConfig config;
                bool parsed = false;
                const bool allowInsecure = false;

                if (str.rfind(get_protocol_scheme(EConfigType::VMESS), 0) == 0) {
                    config = ServerConfig::create(EConfigType::VMESS);
                    if (!config.outboundBean || !config.outboundBean->streamSettings) return std::nullopt;
                    auto& streamSetting = config.outboundBean->streamSettings.value();

                    if (str.find('?') != std::string::npos) {
                        if (!tryResolveVmess4Kitsunebi(str, config)) {
                            return std::nullopt;
                        }
                    } else {
                        std::string result = str.substr(get_protocol_scheme(EConfigType::VMESS).length());
                        result = Utils::decode_base64(result);
                        if (result.empty()) return std::nullopt;

                        auto vmessQRCode = nlohmann::json::parse(result).get<VmessQRCode>();
                        if (vmessQRCode.add.empty() || vmessQRCode.port.empty() || vmessQRCode.id.empty() || vmessQRCode.net.empty()) {
                            return std::nullopt;
                        }

                        config.remarks = vmessQRCode.ps;
                        auto& vnext = config.outboundBean->settings->vnext->at(0);
                        vnext.address = vmessQRCode.add;
                        vnext.port = Utils::parse_int(vmessQRCode.port);
                        vnext.users[0].id = vmessQRCode.id;
                        vnext.users[0].security = vmessQRCode.scy.empty() ? V2rayConfig::DEFAULT_SECURITY : vmessQRCode.scy;
                        vnext.users[0].alterId = Utils::parse_int(vmessQRCode.aid);

                        auto sni = streamSetting.populateTransportSettings(vmessQRCode.net, vmessQRCode.type, vmessQRCode.host, vmessQRCode.path, {}, {}, {}, {}, {});
                        auto fingerprint = vmessQRCode.fp.empty() ? (streamSetting.tlsSettings ? streamSetting.tlsSettings->fingerprint.value_or("") : "") : vmessQRCode.fp;
                        streamSetting.populateTlsSettings(vmessQRCode.tls, allowInsecure, vmessQRCode.sni.empty() ? sni : vmessQRCode.sni, fingerprint, vmessQRCode.alpn, {}, {}, {});
                    }
                    parsed = true;
                } 
                // NEW: Added VLESS parsing logic
                else if (str.rfind(get_protocol_scheme(EConfigType::VLESS), 0) == 0) {
                    std::string full_uri = str.substr(get_protocol_scheme(EConfigType::VLESS).length());
                    
                    std::string remarks;
                    size_t fragment_pos = full_uri.find('#');
                    if (fragment_pos != std::string::npos) {
                        remarks = Utils::url_decode(full_uri.substr(fragment_pos + 1));
                        full_uri = full_uri.substr(0, fragment_pos);
                    }

                    std::string query_str;
                    size_t query_pos = full_uri.find('?');
                    if (query_pos != std::string::npos) {
                        query_str = full_uri.substr(query_pos + 1);
                        full_uri = full_uri.substr(0, query_pos);
                    }
                    auto query_params = Utils::parse_query_params(query_str);

                    size_t at_pos = full_uri.find('@');
                    if (at_pos == std::string::npos) return std::nullopt; // Invalid format

                    std::string user_id = full_uri.substr(0, at_pos);
                    std::string host_port = full_uri.substr(at_pos + 1);

                    std::string host;
                    int port = 0;
                    size_t port_pos = host_port.find_last_of(':');
                    if (port_pos != std::string::npos) {
                        host = host_port.substr(0, port_pos);
                        port = Utils::parse_int(host_port.substr(port_pos + 1));
                    } else {
                        host = host_port;
                    }

                    config = ServerConfig::create(EConfigType::VLESS);
                    auto& streamSetting = config.outboundBean->streamSettings.value();

                    config.remarks = remarks;
                    auto& vnext = config.outboundBean->settings->vnext->at(0);
                    vnext.address = host;
                    vnext.port = port;
                    vnext.users[0].id = user_id;
                    vnext.users[0].encryption = query_params.count("encryption") ? query_params.at("encryption") : "none";
                    vnext.users[0].flow = query_params.count("flow") ? query_params.at("flow") : "";

                    auto get_query_param = [&](const std::string& key) -> std::optional<std::string> {
                        if (query_params.count(key)) return query_params.at(key);
                        return std::nullopt;
                    };

                    auto sni = streamSetting.populateTransportSettings(
                        query_params.count("type") ? query_params.at("type") : "tcp",
                        get_query_param("headerType"),
                        get_query_param("host"),
                        get_query_param("path"),
                        get_query_param("seed"),
                        get_query_param("quicSecurity"),
                        get_query_param("key"),
                        get_query_param("mode"),
                        get_query_param("serviceName")
                    );

                    std::string fingerprint = query_params.count("fp") ? query_params.at("fp") : "";
                    std::string final_sni = query_params.count("sni") ? query_params.at("sni") : sni;
                    
                    streamSetting.populateTlsSettings(
                        query_params.count("security") ? query_params.at("security") : "",
                        allowInsecure,
                        final_sni,
                        fingerprint,
                        query_params.count("alpn") ? query_params.at("alpn") : "",
                        std::nullopt, std::nullopt, std::nullopt
                    );
                    parsed = true;
                }
                else if (str.rfind(get_protocol_scheme(EConfigType::SHADOWSOCKS), 0) == 0) {
                    config = ServerConfig::create(EConfigType::SHADOWSOCKS);
                    std::string result = str.substr(get_protocol_scheme(EConfigType::SHADOWSOCKS).length());

                    size_t remark_pos = result.find('#');
                    if (remark_pos != std::string::npos) {
                        config.remarks = Utils::url_decode(result.substr(remark_pos + 1));
                        result = result.substr(0, remark_pos);
                    }

                    size_t at_pos = result.find('@');
                    if (at_pos != std::string::npos) {
                        result = Utils::decode_base64(result.substr(0, at_pos)) + result.substr(at_pos);
                    } else {
                        result = Utils::decode_base64(result);
                    }

                    std::regex legacy_pattern(R"((.+?):(.*)@(.+?):(\d+))");
                    std::smatch match;
                    if (std::regex_match(result, match, legacy_pattern)) {
                        auto& server = config.outboundBean->settings->servers->at(0);
                        server.method = match[1].str();
                        server.password = match[2].str();
                        server.address = match[3].str();
                        server.port = Utils::parse_int(match[4].str());
                        parsed = true;
                    }
                } 
                // ... Add other protocols (SOCKS, TROJAN) here if needed ...

                if (parsed) {
                    return config;
                }
                return std::nullopt;

            } catch (const std::exception& e) {
                std::cerr << "Error parsing config: " << e.what() << std::endl;
                return std::nullopt;
            }
        }

        inline bool tryResolveVmess4Kitsunebi(const std::string& server, ServerConfig& config) {
            std::string result = server.substr(get_protocol_scheme(EConfigType::VMESS).length());
            size_t query_pos = result.find('?');
            if (query_pos != std::string::npos) {
                result = result.substr(0, query_pos);
            }
            result = Utils::decode_base64(result);

            auto arr1 = Utils::split(result, '@');
            if (arr1.size() != 2) return false;

            auto arr21 = Utils::split(arr1[0], ':');
            auto arr22 = Utils::split(arr1[1], ':');
            if (arr21.size() != 2 || arr22.size() != 2) return false;

            config.remarks = "Kitsunebi/Alien";
            auto& vnext = config.outboundBean->settings->vnext->at(0);
            vnext.address = arr22[0];
            vnext.port = Utils::parse_int(arr22[1]);
            vnext.users[0].id = arr21[1];
            vnext.users[0].security = arr21[0];
            vnext.users[0].alterId = 0;
            return true;
        }
    } // namespace AngConfigManager

    // =================================================================================
    // Full V2Ray Config DTOs
    // =================================================================================
    namespace V2rayFullConfig {
        using json = nlohmann::json;

        // All nested structs (Log, Sniffing, Inbound, etc.) remain unchanged
        // with their NLOHMANN_DEFINE_TYPE_INTRUSIVE macros inside.
        struct Log {
            std::string loglevel = "warning";
            NLOHMANN_DEFINE_TYPE_INTRUSIVE(Log, loglevel);
        };

        struct Sniffing {
            bool enabled = true;
            std::vector<std::string> destOverride = { "http", "tls" };
            NLOHMANN_DEFINE_TYPE_INTRUSIVE(Sniffing, enabled, destOverride);
        };

        struct Inbound {
            std::string tag;
            int port = 0;
            std::string listen = "127.0.0.1";
            std::string protocol;
            std::optional<Sniffing> sniffing;
            std::optional<json> settings;
            NLOHMANN_DEFINE_TYPE_INTRUSIVE(Inbound, tag, port, listen, protocol, sniffing, settings);
        };

        struct RoutingRule {
            std::string type = "field";
            std::optional<std::string> outboundTag;
            std::optional<std::vector<std::string>> inboundTag;
            std::optional<std::vector<std::string>> ip;
            std::optional<std::vector<std::string>> domain;
            std::optional<std::string> port;
            NLOHMANN_DEFINE_TYPE_INTRUSIVE(RoutingRule, type, outboundTag, inboundTag, ip, domain, port);
        };

        struct Routing {
            std::string domainStrategy = "IPIfNonMatch";
            std::optional<std::string> domainMatcher = "mph";
            std::vector<RoutingRule> rules;
            NLOHMANN_DEFINE_TYPE_INTRUSIVE(Routing, domainStrategy, domainMatcher, rules);
        };

        struct DnsServerObject {
            std::string address;
            int port = 53;
            std::optional<std::vector<std::string>> domains;
            std::optional<std::vector<std::string>> expectIPs;
            NLOHMANN_DEFINE_TYPE_INTRUSIVE(DnsServerObject, address, port, domains, expectIPs);
        };

        struct Dns {
            std::optional<std::map<std::string, std::string>> hosts;
            std::optional<std::vector<json>> servers;
            NLOHMANN_DEFINE_TYPE_INTRUSIVE(Dns, hosts, servers);
        };

        struct Fakedns {
            std::string ipPool = "198.18.0.0/15";
            int poolSize = 65535;
            NLOHMANN_DEFINE_TYPE_INTRUSIVE(Fakedns, ipPool, poolSize);
        };

        // FIX: Replaced the macro for the top-level Config struct with explicit functions.
        struct Config {
            Log log;
            std::vector<Inbound> inbounds;
            std::vector<json> outbounds;
            Routing routing;
            std::optional<Dns> dns;
            std::optional<json> stats;
            std::optional<json> policy;
            std::optional<std::vector<Fakedns>> fakedns;
            // NLOHMANN_DEFINE_TYPE_INTRUSIVE macro removed from here.
        };

        // Explicit serialization function (C++ object -> JSON)
        inline void to_json(json& j, const Config& p) {
            j = json{
                {"log", p.log},
                {"inbounds", p.inbounds},
                {"outbounds", p.outbounds},
                {"routing", p.routing}
            };
            // Only add optional fields if they have a value
            if (p.dns.has_value()) j["dns"] = p.dns;
            if (p.stats.has_value()) j["stats"] = p.stats;
            if (p.policy.has_value()) j["policy"] = p.policy;
            if (p.fakedns.has_value()) j["fakedns"] = p.fakedns;
        }

        // Explicit deserialization function (JSON -> C++ object)
        inline void from_json(const json& j, Config& p) {
            // Required fields
            j.at("log").get_to(p.log);
            j.at("inbounds").get_to(p.inbounds);
            j.at("outbounds").get_to(p.outbounds);
            j.at("routing").get_to(p.routing);
            
            // Optional fields
            if (j.contains("dns")) {
                j.at("dns").get_to(p.dns);
            }
            if (j.contains("stats")) {
                j.at("stats").get_to(p.stats);
            }
            if (j.contains("policy")) {
                j.at("policy").get_to(p.policy);
            }
            if (j.contains("fakedns")) {
                j.at("fakedns").get_to(p.fakedns);
            }
        }
    }

    // =================================================================================
    // App-level Constants and Settings
    // =================================================================================
    namespace AppConfigConstants {
        const std::string TAG_AGENT = "proxy";
        const std::string TAG_DIRECT = "direct";
        const std::string TAG_BLOCKED = "block";
        const int PORT_SOCKS = 10808;
        const int PORT_HTTP = 10809;
        const int PORT_LOCAL_DNS = 10853;
    }

    enum class ERoutingMode { GLOBAL_PROXY, BYPASS_LAN, BYPASS_MAINLAND, BYPASS_LAN_MAINLAND, GLOBAL_DIRECT };

    // This struct replaces MMKV settings from the Kotlin code.
    // You must populate this struct with user preferences before calling the generator.
    struct V2rayGeneratorSettings {
        std::string logLevel = "warning";
        int socksPort = AppConfigConstants::PORT_SOCKS;
        int httpPort = AppConfigConstants::PORT_HTTP;
        int localDnsPort = AppConfigConstants::PORT_LOCAL_DNS;
        bool proxySharing = false;
        bool fakeDnsEnabled = false;
        bool sniffingEnabled = true;
        bool localDnsEnabled = false;
        bool speedEnabled = true;
        std::string routingDomainStrategy = "IPIfNonMatch";
        ERoutingMode routingMode = ERoutingMode::GLOBAL_PROXY;
        std::string userRoutingAgent;   // Comma-separated rules
        std::string userRoutingDirect;  // Comma-separated rules
        std::string userRoutingBlocked; // Comma-separated rules
    };


    // =================================================================================
    // V2Ray Config Generator
    // =================================================================================
    namespace V2rayConfigGenerator {
        using namespace V2rayFullConfig;
        using namespace AppConfigConstants;
        using json = nlohmann::json;

        namespace {
            void apply_inbounds_settings(Config& v2rayConfig, const V2rayGeneratorSettings& settings) {
                for (auto& inbound : v2rayConfig.inbounds) {
                    if (!settings.proxySharing) {
                        inbound.listen = "127.0.0.1";
                    }
                }
                if (!v2rayConfig.inbounds.empty()) {
                    v2rayConfig.inbounds[0].port = settings.socksPort;
                    if (v2rayConfig.inbounds[0].sniffing) {
                        v2rayConfig.inbounds[0].sniffing->enabled = settings.fakeDnsEnabled || settings.sniffingEnabled;
                        if (!settings.sniffingEnabled) {
                            v2rayConfig.inbounds[0].sniffing->destOverride.clear();
                        }
                        if (settings.fakeDnsEnabled) {
                            auto& overrides = v2rayConfig.inbounds[0].sniffing->destOverride;
                            if (std::find(overrides.begin(), overrides.end(), "fakedns") == overrides.end()) {
                                overrides.push_back("fakedns");
                            }
                        }
                    }
                }
                if (v2rayConfig.inbounds.size() > 1) {
                    v2rayConfig.inbounds[1].port = settings.httpPort;
                }
            }

            void apply_fakedns(Config& v2rayConfig, const V2rayGeneratorSettings& settings) {
                if (settings.fakeDnsEnabled) {
                    v2rayConfig.fakedns = std::vector<Fakedns>{ {} };
                    for (auto& outbound_json : v2rayConfig.outbounds) {
                        if (outbound_json.is_object() && outbound_json.value("protocol", "") == "freedom") {
                            outbound_json["settings"]["domainStrategy"] = "UseIP";
                        }
                    }
                }
            }
            
            void routing_user_rule(const std::string& userRule, const std::string& tag, Config& v2rayConfig) {
                if (userRule.empty()) return;
                RoutingRule rulesDomain, rulesIP;
                rulesDomain.outboundTag = tag;
                rulesIP.outboundTag = tag;
                rulesDomain.domain = std::vector<std::string>();
                rulesIP.ip = std::vector<std::string>();

                auto rules = Utils::split(userRule, ',');
                for (const auto& rule : rules) {
                    if (rule.empty()) continue;
                    std::string trimmed_rule = std::regex_replace(rule, std::regex("^\\s+|\\s+$"), "");
                    if (Utils::is_ip_address(trimmed_rule) || trimmed_rule.rfind("geoip:", 0) == 0) {
                        rulesIP.ip->push_back(trimmed_rule);
                    } else {
                        rulesDomain.domain->push_back(trimmed_rule);
                    }
                }
                if (rulesDomain.domain && !rulesDomain.domain->empty()) v2rayConfig.routing.rules.push_back(rulesDomain);
                if (rulesIP.ip && !rulesIP.ip->empty()) v2rayConfig.routing.rules.push_back(rulesIP);
            }

            void routing_geo(const std::string& ipOrDomain, const std::string& code, const std::string& tag, Config& v2rayConfig) {
                if (code.empty()) return;
                if (ipOrDomain == "ip" || ipOrDomain.empty()) {
                    RoutingRule rule;
                    rule.outboundTag = tag;
                    rule.ip = { "geoip:" + code };
                    v2rayConfig.routing.rules.push_back(rule);
                }
                if (ipOrDomain == "domain" || ipOrDomain.empty()) {
                    RoutingRule rule;
                    rule.outboundTag = tag;
                    rule.domain = { "geosite:" + code };
                    v2rayConfig.routing.rules.push_back(rule);
                }
            }

            void apply_routing(Config& v2rayConfig, const V2rayGeneratorSettings& settings) {
                routing_user_rule(settings.userRoutingAgent, TAG_AGENT, v2rayConfig);
                routing_user_rule(settings.userRoutingDirect, TAG_DIRECT, v2rayConfig);
                routing_user_rule(settings.userRoutingBlocked, TAG_BLOCKED, v2rayConfig);

                v2rayConfig.routing.domainStrategy = settings.routingDomainStrategy;
                
                RoutingRule googleapisRoute;
                googleapisRoute.outboundTag = TAG_AGENT;
                googleapisRoute.domain = { "domain:googleapis.cn" };

                switch (settings.routingMode) {
                    case ERoutingMode::BYPASS_LAN:
                        routing_geo("ip", "private", TAG_DIRECT, v2rayConfig);
                        break;
                    case ERoutingMode::BYPASS_MAINLAND:
                        routing_geo("", "cn", TAG_DIRECT, v2rayConfig);
                        v2rayConfig.routing.rules.insert(v2rayConfig.routing.rules.begin(), googleapisRoute);
                        break;
                    case ERoutingMode::BYPASS_LAN_MAINLAND:
                        routing_geo("ip", "private", TAG_DIRECT, v2rayConfig);
                        routing_geo("", "cn", TAG_DIRECT, v2rayConfig);
                        v2rayConfig.routing.rules.insert(v2rayConfig.routing.rules.begin(), googleapisRoute);
                        break;
                    // FIX: Added curly braces to create a scope for the variable declaration
                    case ERoutingMode::GLOBAL_DIRECT: {
                        RoutingRule globalDirect;
                        globalDirect.outboundTag = TAG_DIRECT;
                        globalDirect.port = "0-65535";
                        v2rayConfig.routing.rules.push_back(globalDirect);
                        break;
                    }
                    case ERoutingMode::GLOBAL_PROXY:
                    default:
                        break;
                }
            }

            // ... (dns and customLocalDns are complex and would be implemented here) ...
        }

        // Main public function to generate the config
        inline std::optional<std::string> generate(const ServerConfig& serverConfig, const V2rayGeneratorSettings& settings) {
            try {
                if (serverConfig.configType == EConfigType::CUSTOM) {
                    // For custom configs, we assume the full config is already provided.
                    // This part of the logic would need to be adapted based on how you store custom configs.
                    return "{\"error\":\"Custom config generation not implemented in this translation.\"}";
                }

                // 1. Load the base config from an embedded string
                const char* base_config_str = R"({
                "stats":{},
                "log": {
                    "loglevel": "warning"
                },
                "policy":{
                    "levels": {
                        "8": {
                        "handshake": 4,
                        "connIdle": 300,
                        "uplinkOnly": 1,
                        "downlinkOnly": 1
                        }
                    },
                    "system": {
                        "statsOutboundUplink": true,
                        "statsOutboundDownlink": true
                    }
                },
                "inbounds": [{
                    "tag": "socks",
                    "port": 10808,
                    "listen": "127.0.0.1",
                    "protocol": "socks",
                    "settings": {
                    "auth": "noauth",
                    "udp": true,
                    "userLevel": 8
                    },
                    "sniffing": {
                    "enabled": true,
                    "destOverride": [
                        "http",
                        "tls"
                    ]
                    }
                },
                {
                    "tag": "http",
                    "port": 10809,
                    "listen": "127.0.0.1",
                    "protocol": "http",
                    "settings": {
                    "userLevel": 8
                    },
                    "sniffing": {
                    "enabled": true,
                    "destOverride": [
                        "http",
                        "tls"
                    ]
                    }
                }
                ],
                "outbounds": [
                {
                    "protocol": "freedom",
                    "settings": {},
                    "tag": "direct"
                },
                {
                    "protocol": "blackhole",
                    "tag": "block",
                    "settings": {
                    "response": {
                        "type": "http"
                    }
                    }
                }
                ],
                "routing": {
                    "domainStrategy": "IPIfNonMatch",
                    "domainMatcher": "mph",
                    "rules": []
                },
                "dns": {
                    "hosts": {},
                    "servers": []
                }
                }
                )";
                json v2ray_json = json::parse(base_config_str);
                Config v2rayConfig = v2ray_json.get<Config>();
               // [json.exception.out_of_range.403] key 'loglevel' not found
                // 2. Set log level
                v2rayConfig.log.loglevel = settings.logLevel;

                // 3. Apply inbound settings
                apply_inbounds_settings(v2rayConfig, settings);

                // 4. Prepare the main proxy outbound
                json outbound_proxy = serverConfig.outboundBean; // Use nlohmann's automatic conversion
                outbound_proxy["tag"] = TAG_AGENT;
                v2rayConfig.outbounds.insert(v2rayConfig.outbounds.begin(), outbound_proxy);

                // 5. Apply routing, DNS, etc.
                apply_routing(v2rayConfig, settings);
                apply_fakedns(v2rayConfig, settings);
                // Note: Full DNS and customLocalDns logic is complex and omitted for brevity,
                // but would be called here.

                // 6. Handle stats/policy
                if (!settings.speedEnabled) {
                    v2rayConfig.stats = nullptr;
                    v2rayConfig.policy = nullptr;
                }

                // 7. Serialize to string
                return json(v2rayConfig).dump(2); // 2-space indentation

            } catch (const std::exception& e) {
                std::cerr << "Error generating V2Ray config: " << e.what() << std::endl;
                return std::nullopt;
            }
        }
    } // namespace V2rayConfigGenerator

} // namespace V2rayConfigWin
