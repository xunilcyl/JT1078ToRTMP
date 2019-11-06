#include "ServerManager.h"
#include <assert.h>
#include "Logger.h"
#include "ServerAllocator.h"

constexpr int TIMER_INTERVAL = 10;

ServerManager::ServerManager()
    : m_serverAllocator(new ServerAllocator)
{    
}

ServerManager::~ServerManager()
{
    // m_serverManager.Uninit();
}

void ServerManager::Run()
{
    boost::asio::steady_timer activeTimer(m_ioService, boost::asio::chrono::seconds(TIMER_INTERVAL));
    activeTimer.async_wait(
        [this](boost::system::error_code ec, boost::asio::steady_timer* t){
            OnTimeout(ec, t);
        });
    m_ioService.run();
}

void ServerManager::OnTimeout()
{
    std::cout << "timeout" << std::endl;
    activeTimer.async_wait(
        [this](boost::system::error_code ec, boost::asio::steady_timer* t){
            OnTimeout(ec, t);
        });
}

int ServerManager::Init()
{
    m_thread.reset(
        new std::thread(
            [this]() {
                this->Run();
            }));
    return 0;
}

void ServerManager::UnInit()
{

}

int ServerManager::CreateServer(const std::string& uniqueID, std::string& ip, int& port)
{
    if (uniqueID.empty()) {
        LOG_WARN << "uniqueID is empty!";
        return RET_INVALID_PARAMS;
    }

    assert(m_serverAllocator != nullptr);

    auto server = m_serverAllocator.Find(uniqueID);
    if (server) {
        ip = server->GetIP();
        port = server->GetPort();
        LOG_WARN << "there is a server running for " << uniqueID;
        return RET_SERVER_ALREADY_RUNNING;
    }
    


    return RET_SUCCESS;
}

int ServerManager::DestroyServer(const std::string& uniqueID)
{
    return 0;
}