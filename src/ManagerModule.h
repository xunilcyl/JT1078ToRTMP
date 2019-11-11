#pragma once
#include <memory>
#include "IMediaServerManager.h"
#include "IHttpServer.h"

typedef std::unique_ptr<IMediaServerManager> MediaServerManagerPtr;
typedef std::unique_ptr<IHttpServer> HttpServerPtr;

class ManagerModule
{
public:
    ManagerModule();

    int Start();
    int Run();
    int Stop();

private:
    MediaServerManagerPtr m_mediaServerManager;
    HttpServerPtr m_httpServer;
};