#pragma once

class IMediaDataCallback
{
public:
    virtual ~IMediaDataCallback() = default;
    virtual void OnData(const char* data, int size) = 0;
};