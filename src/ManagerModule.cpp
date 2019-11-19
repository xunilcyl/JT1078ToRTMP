#include "ManagerModule.h"
#include "HttpServer.h"
#include "HttpNotifier.h"
#include "MediaServerManager.h"

ManagerModule::ManagerModule()
    : m_notifier(new HttpNotifier)
    , m_mediaServerManager(new MediaServerManager(*m_notifier))
    , m_httpServer(new HttpServer(*m_mediaServerManager, *m_notifier))
{
}

int ManagerModule::Start()
{
    if (m_notifier->Start() < 0 ||
        m_mediaServerManager->Start() < 0 ||
        m_httpServer->Start() < 0) {
        return -1;
    }
    return 0;
}

int ManagerModule::Run()
{
    return getchar();
}

int ManagerModule::Stop()
{
    m_notifier->Stop();
    m_httpServer->Stop();
    m_mediaServerManager->Stop();
    return 0;
}