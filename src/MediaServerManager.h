#pragma once
#include "IAudioSenderEngine.h"
#include "IMediaServerManager.h"
#include "IMediaServer.h"
#include "IPortAllocator.h"
#include "IRtmpClient.h"
#include <boost/asio.hpp>
#include <string>
#include <thread>

class INotifier;

typedef std::unique_ptr<IMediaServer> MediaServerPtr;
typedef std::unique_ptr<IRtmpClient> RtmpClientPtr;

struct MediaServerInfo
{
    MediaServerPtr m_mediaServer;
    RtmpClientPtr m_rtmpClient;
};

typedef std::map<int, MediaServerInfo> MediaServerMap;

class MediaServerManager : public IMediaServerManager
{
public:
    MediaServerManager(INotifier& notifier);

    int Start() override;
    void Stop() override;
    int GetPort(const std::string& uniqueID) override;
    int FreePort(int port) override;

private:
    int Run();

private:
    boost::asio::io_context m_ioContext;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> m_work;
    std::unique_ptr<std::thread> m_mediaThread;
    std::unique_ptr<IPortAllocator> m_portAllocator;
    std::unique_ptr<IAudioSenderEngine> m_audioSenderEngine;
    MediaServerMap m_mediaServerMap;
    INotifier& m_notifier;
};