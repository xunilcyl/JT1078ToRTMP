#pragma once

class IMediaDataCallback
{
public:
    virtual ~IMediaDataCallback() = default;
    virtual void OnData(const char* data, int size, bool isEndOfFrame) = 0;
    virtual void OnAudioData(const char* data, int size, bool isEndOfFrame) = 0;
};