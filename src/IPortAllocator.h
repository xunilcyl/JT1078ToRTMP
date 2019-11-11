#pragma once

class IPortAllocator
{
public:
    virtual ~IPortAllocator() = default;
    virtual int AllocatePort() = 0;
    virtual void FreePort(int port) = 0;
};