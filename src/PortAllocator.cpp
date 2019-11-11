#include "PortAllocator.h"
#include <algorithm>
#include "Logger.h"

constexpr int MIN_PORT_NUMBER = 30000;
constexpr int MAX_PORT_NUMBER = 40000;

PortAllocator::PortAllocator()
{
    for (int i = MIN_PORT_NUMBER; i <= MAX_PORT_NUMBER; ++i) {
        m_unusedPorts.push_back(i);
    }
    m_usedPorts.clear();
}

PortAllocator::~PortAllocator()
{
    m_unusedPorts.clear();
    m_usedPorts.clear();
}

int PortAllocator::AllocatePort()
{
    auto port = m_unusedPorts.front();
    m_unusedPorts.pop_front();

    m_usedPorts.push_back(port);
    LOG_INFO << "allocate port " << port;
    return port;
}

void PortAllocator::FreePort(int port)
{
    auto it = std::find(m_usedPorts.begin(), m_usedPorts.end(), port);
    if (it != m_usedPorts.end()) {
        m_usedPorts.erase(it);
        m_unusedPorts.push_back(port);

        LOG_INFO << "free port " << port;
    }
}