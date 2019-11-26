#pragma once
#include "IConfiguration.h"

constexpr int DEFAULT_FPS = 25;

class Configuration : public IConfiguration
{
public:
    int Init() override;
    void UnInit() override;
    std::string GetPublicIP() override;
    std::string GetHttpNotifyUrl() override;
    long GetMaxAnalyzeDuration() override;
    bool IfUseLibrtmp() override;
    int GetFps() override;

private:
    int Parse(const char* file);
    void CheckConfigs();

private:
    std::string m_publicIP;
    std::string m_httpNotifyUrl;
    long m_maxAnalyzeDuration = 0;
    bool m_ifUseLibrtmp = false;
    int m_fps = DEFAULT_FPS;
};