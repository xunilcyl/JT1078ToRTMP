#pragma once
#include "IMediaDataCallback.h"

class IRtmpListener
{
public:
    virtual ~IRtmpListener() = default;

    virtual void OnRtmpError(int error) = 0;
};

class IRtmpClient : public IMediaDataCallback
{
public:
    virtual ~IRtmpClient() = default;

    virtual int Start() = 0;
    virtual int Stop() = 0;
};