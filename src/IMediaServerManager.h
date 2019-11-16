#pragma once
#include <string>

class IMediaServerManager
{
public:
    ~IMediaServerManager() = default;

    virtual int Start() = 0;
    virtual void Stop() = 0;
    virtual int GetPort(const std::string& uniqueID) = 0;
    virtual int FreePort(int port) = 0;
};
