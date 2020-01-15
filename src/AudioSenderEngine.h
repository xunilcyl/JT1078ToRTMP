#pragma once
#include "IAudioSenderEngine.h"
#include <map>
#include <mutex>
#include <thread>

class AudioSenderEngine : public IAudioSenderEngine
{
public:
    int Start() override;
    void Stop() override;
    int AddSender(IAudioSender* sender) override;
    void RemoveSender(IAudioSender* sender) override;

private:
    void Run();

private:
    std::unique_ptr<std::thread> m_thread;
    std::mutex m_mutex;
    std::map<IAudioSender*, bool> m_senders;
    bool m_stop = false;
};