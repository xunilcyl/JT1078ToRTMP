#pragma once

class IMediaServer
{
public:
    ~IMediaServer() = default;

    virtual int Start() = 0;
    virtual int Run() = 0;
    virtual void Stop() = 0;
    virtual int GetPort() = 0;
};
