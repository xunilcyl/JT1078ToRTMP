#pragma once

class IMediaSession
{
public:
    virtual ~IMediaSession() = default;

    virtual int Start() = 0;
    virtual void Stop() = 0;
};