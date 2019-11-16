#pragma once

class IMediaSessionListener
{
public:
    virtual ~IMediaSessionListener() = default;
    virtual void OnError(const char* msg) = 0;
};

class IMediaSession
{
public:
    virtual ~IMediaSession() = default;

    virtual int Start() = 0;
    virtual void Stop() = 0;
};