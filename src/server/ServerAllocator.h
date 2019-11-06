#pragma once

#include <map>
#include <string>

#include "IServer.h"

class ServerAllocator
{
public:
    ServerAllocator();
    ~ServerAllocator();
    
    int CreateAndStartServer(const std::string& uniqueID);
    int StopServer();
    ServerPtr Find(const std::string& uniqueID);

private:
    std::map<std::string, ServerPtr> m_runningServers;
};