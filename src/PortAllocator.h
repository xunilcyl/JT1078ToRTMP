#pragma once

#include "IPortAllocator.h"
#include <deque>
#include <list>

class PortAllocator : public IPortAllocator
{
public:
    PortAllocator();
    ~PortAllocator();

    int AllocatePort() override;
    void FreePort(int port) override;

private:
    std::deque<int> m_unusedPorts;
    std::list<int> m_usedPorts;
};