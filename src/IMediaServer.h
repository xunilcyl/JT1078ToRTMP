#pragma once

class IMediaServer
{
public:
    virtual ~IMediaServer() = default;
    virtual int Start(int port) = 0;
    virtual int Stop() = 0;
};