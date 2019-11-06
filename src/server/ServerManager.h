#pragma once

#include <map>
#include <string>
#include <thread>
#include "IServer.h"

class ServerAllocator;
using ServerAllocatorPtr = std::unique_ptr<ServerAllocator>;
using ThreadPtr = std::unique_ptr<std::thread>;

class ServerManager
{
public:
    ServerManager();
    ~ServerManager();
    int Init();
    void UnInit();
    int CreateServer(const std::string& uniqueID, std::string& ip, int& port);
    int DestroyServer(const std::string& uniqueID);

private:
    void Run();
    
private:
    boost::asio::io_service m_ioService;
    ThreadPtr m_thread;
    ServerAllocatorPtr m_serverAllocator;
};