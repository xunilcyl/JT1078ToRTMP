#pragma once

class IMediaServerManager
{
public:
    ~IMediaServerManager() = default;

    virtual int Start() = 0;
    virtual void Stop() = 0;
    virtual int GetPort() = 0;
    virtual int FreePort(int port) = 0;
};
