#include "ServerAllocator.h"
#include "Logger.h"
#include "ServerManager.h"

ServerAllocator::ServerAllocator()
{    
}

ServerAllocator::~ServerAllocator()
{
}

int ServerAllocator::CreateAndStartServer(const std::string& uniqueID)
{

}

int ServerAllocator::StopServer()
{

}

ServerPtr ServerAllocator::Find(const std::string& uniqueID)
{
    auto it = m_runningServers.find(uniqueID);
    return it != m_runningServers.end() ? *it : nullptr;
}