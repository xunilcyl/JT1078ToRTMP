#pragma once
#include <memory>
#include "IHttpServer.h"
#include "IMediaServerManager.h"
#include "INotifier.h"

typedef std::unique_ptr<IMediaServerManager> MediaServerManagerPtr;
typedef std::unique_ptr<IHttpServer> HttpServerPtr;
typedef std::unique_ptr<INotifier> NotifierPtr;

class ManagerModule
{
public:
    ManagerModule();

    int Start();
    int Run();
    int Stop();

private:
    NotifierPtr m_notifier;
    MediaServerManagerPtr m_mediaServerManager;
    HttpServerPtr m_httpServer;
};