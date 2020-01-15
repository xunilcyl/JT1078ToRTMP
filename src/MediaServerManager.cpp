#include "MediaServerManager.h"
#include "AudioSenderEngine.h"
#include "Logger.h"
#include "MediaServer.h"
#include "PortAllocator.h"
#include "JT1078MediaParser.h"
#include "RtmpClient.h"

// Try to allocate media server port serveral times if failed
constexpr int MAX_TRY_COUNT = 100;

MediaServerManager::MediaServerManager(INotifier& notifier)
    : m_work(boost::asio::make_work_guard(m_ioContext))
    , m_portAllocator(new PortAllocator)
    , m_notifier(notifier)
{
}

int MediaServerManager::Start()
{
    LOG_INFO << "Start media server manager";
    m_audioSenderEngine.reset(new AudioSenderEngine);
    m_audioSenderEngine->Start();
    m_mediaThread.reset(new std::thread(&MediaServerManager::Run, this));

    if (m_mediaThread == nullptr) {
        return -1;
    }

    return 0;
}

int MediaServerManager::Run()
{
    LOG_INFO << "Media server manager is running";
    auto ret = m_ioContext.run();
    LOG_INFO << "Media server manager run finished";

    return ret;
}

void MediaServerManager::Stop()
{
    m_ioContext.stop();
    m_mediaThread->join();
    m_audioSenderEngine->Stop();

    LOG_INFO << "Media server manager is stopped";
}

int MediaServerManager::GetPort(const std::string& uniqueID)
{
    RtmpClientPtr rtmpClient(new RtmpClient(uniqueID, m_notifier, *m_audioSenderEngine));
    rtmpClient->Start();
    
    std::unique_ptr<IMediaParser> mediaParser(new JT1078MediaParser);
    MediaServerPtr mediaServer(new MediaServer(m_ioContext, std::move(mediaParser), *rtmpClient, m_notifier, uniqueID));

    for (int i = 0; i < MAX_TRY_COUNT; ++i) {
        int port = m_portAllocator->AllocatePort();
        if (mediaServer->Start(port) == 0) {
            m_mediaServerMap.emplace(port, MediaServerInfo{std::move(mediaServer), std::move(rtmpClient)});
            return port;
        }
        else {
            m_portAllocator->FreePort(port);
        }
    }

    LOG_ERROR << "Try " << MAX_TRY_COUNT << " times, but can't allocate port";

    return -1;
}

int MediaServerManager::FreePort(int port)
{
    auto iter = m_mediaServerMap.find(port);
    if (iter == m_mediaServerMap.end()) {
        LOG_INFO << "No corresponding media server for port " << port;
        return -1;
    }

    auto ret = iter->second.m_mediaServer->Stop();
    iter->second.m_rtmpClient->Stop();
    m_mediaServerMap.erase(iter);

    return ret;
}
