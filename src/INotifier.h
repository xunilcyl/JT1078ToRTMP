#pragma once
#include <string>

class INotifier
{
public:
    virtual ~INotifier() = default;
    virtual int Start() = 0;
    virtual void Stop() = 0;
    virtual void NotifyRtmpPlay(const std::string& uniqueID) = 0;
    virtual void NotifyRtmpStop(const std::string& uniqueID) = 0;
    virtual void NotifyDeviceDisconnect(const std::string& uniqueID) = 0;
};