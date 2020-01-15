#include "AudioSenderEngine.h"
#include "Logger.h"
#include <chrono>

int AudioSenderEngine::Start()
{
    m_thread.reset(new std::thread(&AudioSenderEngine::Run, this));
    return 0;
}

void AudioSenderEngine::Stop()
{
    m_stop = true;
    m_thread->join();
}

int AudioSenderEngine::AddSender(IAudioSender* sender)
{
    if (sender == nullptr) {
        return -1;
    }

    LOG_INFO << "send: " << sender;

    std::lock_guard<std::mutex> guard(m_mutex);
    m_senders[sender] = true;

    return 0;
}

void AudioSenderEngine::RemoveSender(IAudioSender* sender)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    auto iter = m_senders.find(sender);
    if (iter != m_senders.end()) {
        m_senders.erase(iter);

        LOG_INFO << "sender: " << sender;
    }
}

void AudioSenderEngine::Run()
{
    // TODO: only when there are senders need to perform send operator, otherwise wait
    while (!m_stop) {
        std::lock_guard<std::mutex> guard(m_mutex);
        for (auto& sender : m_senders) {
            sender.first->SendAudio();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}