#pragma once
#include "IAudioSender.h"

class IAudioSenderEngine
{
public:
    virtual ~IAudioSenderEngine() = default;

    virtual int Start() = 0;
    virtual void Stop() = 0;
    virtual int AddSender(IAudioSender* sender) = 0;
    virtual void RemoveSender(IAudioSender* sender) = 0;
};