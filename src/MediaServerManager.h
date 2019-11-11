#pragma once
#include "IMediaServerManager.h"
#include "IMediaServer.h"
#include "IPortAllocator.h"
#include <boost/asio.hpp>
#include <thread>

typedef std::unique_ptr<IMediaServer> MediaServerPtr;
typedef std::map<int, MediaServerPtr> MediaServerMap;

class MediaServerManager : public IMediaServerManager
{
public:
    MediaServerManager();

    int Start() override;
    void Stop() override;
    int GetPort() override;
    int FreePort(int port) override;

private:
    int Run();

private:
    boost::asio::io_context m_ioContext;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> m_work;
    std::unique_ptr<std::thread> m_mediaThread;
    std::unique_ptr<IPortAllocator> m_portAllocator;
    MediaServerMap m_mediaServerMap;
};