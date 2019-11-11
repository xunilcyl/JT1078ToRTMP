#include "MediaServerManager.h"
#include "Logger.h"
#include "MediaServer.h"
#include "PortAllocator.h"

MediaServerManager::MediaServerManager()
    : m_work(boost::asio::make_work_guard(m_ioContext))
    , m_portAllocator(new PortAllocator)
{
}

int MediaServerManager::Start()
{
    LOG_INFO << "Start media server";
    m_mediaThread.reset(new std::thread(&MediaServerManager::Run, this));
    return 0;
}

int MediaServerManager::Run()
{
    LOG_INFO << "Media server is running";
    auto ret = m_ioContext.run();
    LOG_INFO << "Media server run finished";
    return ret;
}

void MediaServerManager::Stop()
{
    LOG_INFO << "Stop media server";
    m_ioContext.stop();
    m_mediaThread->join();
}

int MediaServerManager::GetPort()
{
    constexpr int MAX_TRY_COUNT = 10;
    MediaServerPtr mediaServer(new MediaServer(m_ioContext));

    for (int i = 0; i < MAX_TRY_COUNT; ++i) {
        int port = m_portAllocator->AllocatePort();
        if (mediaServer->Start(port) == 0) {
            m_mediaServerMap.emplace(port, std::move(mediaServer));
            return port;
        }
        else {
            m_portAllocator->FreePort(port);
        }
    }
    return -1;
}

int MediaServerManager::FreePort(int port)
{
    auto iter = m_mediaServerMap.find(port);
    if (iter == m_mediaServerMap.end()) {
        LOG_INFO << "No corresponding media server for port " << port;
        return -1;
    }

    auto ret = iter->second->Stop();
    m_mediaServerMap.erase(iter);

    return ret;
}
