#include "ManagerModule.h"
#include "HttpServer.h"
//#include "MediaServerManagerMock.h"
#include "MediaServerManager.h"

ManagerModule::ManagerModule()
    //: m_mediaServerManager(new MediaServerManagerMock)
    : m_mediaServerManager(new MediaServerManager)
    , m_httpServer(new HttpServer(*m_mediaServerManager))
{
}

int ManagerModule::Start()
{
    m_mediaServerManager->Start();
    m_httpServer->Start();
    return 0;
}

int ManagerModule::Run()
{
    return getchar();
    //return m_mediaServerManager->Run();
}

int ManagerModule::Stop()
{
    m_httpServer->Stop();
    m_mediaServerManager->Stop();
    return 0;
}