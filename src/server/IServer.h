#pragma once

#include <memory>
#include <string>

class IServer
{
public:
    ~IServer() = default;

    virtual int Start() = 0;
    virtual void Stop() = 0;
    virtual std::string GetIP() = 0;
    virtual int GetPort() = 0;
};

using ServerPtr = std::shared_ptr<IServer>;