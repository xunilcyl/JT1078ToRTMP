#pragma once

class IAudioSender
{
public:
    virtual ~IAudioSender() = default;
    virtual void SendAudio() = 0;
};