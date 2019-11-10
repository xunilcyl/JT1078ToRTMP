#pragma once
#include <memory>
#include "IMediaServer.h"
#include "IHttpServer.h"

typedef std::unique_ptr<IMediaServer> MediaServerPtr;
typedef std::unique_ptr<IHttpServer> HttpServerPtr;

class ManagerModule
{
public:
    ManagerModule();

    int Start();
    int Run();
    int Stop();

private:
    MediaServerPtr m_mediaServer;
    HttpServerPtr m_httpServer;
};