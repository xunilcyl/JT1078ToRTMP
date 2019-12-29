#pragma once
#include <string>


class IConfiguration
{

public:
    static IConfiguration& Get();
    
    virtual ~IConfiguration() = default;
    virtual int Init() = 0;
    virtual void UnInit() = 0;
    virtual std::string GetPublicIP() = 0;
    virtual std::string GetHttpNotifyUrl() = 0;
    virtual long GetMaxAnalyzeDuration() = 0;
    virtual bool IfUseLibrtmp() = 0;
    virtual int GetFps() = 0;
    virtual bool IsCounterEnabled() = 0;
};