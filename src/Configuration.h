#pragma once
#include "IConfiguration.h"

class Configuration : public IConfiguration
{
public:
    int Init() override;
    void UnInit() override;
    std::string GetPublicIP() override;
    std::string GetHttpNotifyUrl() override;
    long GetMaxAnalyzeDuration() override;

private:
    int Parse(const char* file);

private:
    std::string m_publicIP;
    std::string m_httpNotifyUrl;
    long m_maxAnalyzeDuration = 0;
};