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
    virtual std::string getHttpNotifyUrl() = 0;
};