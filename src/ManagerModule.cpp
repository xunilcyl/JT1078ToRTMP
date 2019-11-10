#include "ManagerModule.h"
#include "HttpServer.h"
#include "MediaServerMock.h"

ManagerModule::ManagerModule()
    : m_mediaServer(new MediaServerMock)
    , m_httpServer(new HttpServer(*m_mediaServer))
{
}

int ManagerModule::Start()
{
    m_mediaServer->Start();
    m_httpServer->Start();
    return 0;
}

int ManagerModule::Run()
{
    return m_mediaServer->Run();
}

int ManagerModule::Stop()
{
    m_httpServer->Stop();
    m_mediaServer->Stop();
    return 0;
}