#pragma once
#include <string>

class IMediaCounter
{
public:
    static IMediaCounter& Get();

public:
    virtual ~IMediaCounter() = default;
    virtual int Register(const std::string& uniqueID) = 0;
    virtual void UnRegister(const std::string& uniqueID) = 0;
    virtual void UpdateCounter(const std::string& uniqueID, long size) = 0;
};

