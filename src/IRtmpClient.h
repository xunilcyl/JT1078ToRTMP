#pragma once
#include "IMediaDataCallback.h"

class IRtmpClient : public IMediaDataCallback
{
public:
    virtual ~IRtmpClient() = default;

    virtual int Start() = 0;
    virtual int Stop() = 0;
};