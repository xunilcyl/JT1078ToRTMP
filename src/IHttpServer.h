#pragma once

class IHttpServer
{
public:
    virtual ~IHttpServer() = default;
    virtual int Start() = 0;
    virtual int Stop() = 0;
};