#include "Configuration.h"
#include "Logger.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <cassert>

using namespace boost::property_tree;

constexpr char CONFIG_FILE[] = "config.json";

constexpr char PUBLIC_IP[] = "public_ip";
constexpr char HTTP_NOTIFY_URL[] = "http_notify_url";
constexpr char MAX_ANALYZE_DURATION[] = "max_analyze_duration";
constexpr char USE_LIBRTMP[] = "use_librtmp";
constexpr char FPS[] = "fps";

static Configuration s_configuration;

IConfiguration& IConfiguration::Get()
{
    return s_configuration;
}

int Configuration::Init()
{
    return Parse(CONFIG_FILE);
}

void Configuration::UnInit()
{

}

std::string Configuration::GetPublicIP()
{
    return m_publicIP;
}

std::string Configuration::GetHttpNotifyUrl()
{
    return m_httpNotifyUrl;
}

long Configuration::GetMaxAnalyzeDuration()
{
    return m_maxAnalyzeDuration;
}

bool Configuration::IfUseLibrtmp()
{
    return m_ifUseLibrtmp;
}

int Configuration::GetFps()
{
    return m_fps;
}

int Configuration::Parse(const char* file)
{
    assert(file);

    LOG_INFO << "Parsing config file " << file;

    std::ifstream ss(file);
    if (!ss) {
        LOG_ERROR << "Open config file " << file << " failed";
        return -1;
    }

    ptree pt;

    try {
        json_parser::read_json(ss, pt);
		m_publicIP = pt.get<std::string>(PUBLIC_IP);
		m_httpNotifyUrl = pt.get<std::string>(HTTP_NOTIFY_URL);
        m_maxAnalyzeDuration = pt.get<long>(MAX_ANALYZE_DURATION);
        m_ifUseLibrtmp = pt.get<bool>(USE_LIBRTMP);
        m_fps = pt.get<int>(FPS);

        if (m_publicIP.empty() ||
            m_httpNotifyUrl.empty()) {
            LOG_ERROR << "Mandatory field should not be empty";
            ss.close();
            return -1;
        }
        CheckConfigs();
    }
    catch (ptree_error& e) {
        LOG_ERROR << "Parse config file error: " << e.what();
        ss.close();
        return -1;
    }

    LOG_INFO << "Parse config file done.";

    return 0;
}

void Configuration::CheckConfigs()
{
    if (m_fps <= 0 || m_fps > 120) {
        LOG_WARN << "Invalid fps " << m_fps << ", set to default fps " << DEFAULT_FPS;
        m_fps = DEFAULT_FPS;
    }
}