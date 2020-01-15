#include "MediaCounter.h"
#include "Logger.h"
#include "IConfiguration.h"
#include <chrono>

//static
IMediaCounter& IMediaCounter::Get()
{
    static MediaCounter g_mediaCounter;
    return g_mediaCounter;
}

MediaCounter::MediaCounter()
{
    Start();
}

MediaCounter::~MediaCounter()
{
    Stop();
}

void MediaCounter::Start()
{
    if (IConfiguration::Get().IsCounterEnabled()) {
        LOG_INFO << "debug counter enabled.";

        m_thread.reset(new std::thread(&MediaCounter::Run, this));
        m_stopped = false;
    }
}

void MediaCounter::Stop()
{
    if (m_thread) {
        m_stopped = true;
        m_thread->join();
        m_thread.reset();
    }
    for (auto& counter : m_counters) {
        if (counter.second.m_file) {
            fclose(counter.second.m_file);
        }
    }
}

void MediaCounter::Run()
{
    while (true) {
        if (m_stopped) {
            break;
        }

        {
            std::lock_guard<std::mutex> guard(m_lock);
            auto curTime = time(0);
            for (auto& counter : m_counters) {
                StoreRecord(counter.second, curTime);
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void MediaCounter::StoreRecord(CounterInfo& info, time_t curTime)
{
    char buf[64] = {0};
    snprintf(buf, sizeof(buf) - 1, "%ld %ld\n", curTime, info.m_count);
    fwrite(buf, 1, strlen(buf), info.m_file);
}

int MediaCounter::Register(const std::string& uniqueID)
{
    if (m_stopped) {
        return -1;
    }

    if (uniqueID.empty()) {
        LOG_ERROR << "uniqueID is empty";
        return -1;
    }

    LOG_INFO << "register uniqueID: " << uniqueID;

    std::string filename = "counter/" + uniqueID + ".ct";
    CounterInfo ci;
    ci.m_count = 0;
    ci.m_file = fopen(filename.c_str(), "a+");
    if (ci.m_file == NULL) {
        LOG_ERROR << "open file failed";
        return -1;
    }

    std::lock_guard<std::mutex> guard(m_lock);
    auto result = m_counters.insert({uniqueID, ci});
    if (!result.second) {
        LOG_ERROR << "insert failed.";
        fclose(ci.m_file);
        return -1;
    }

    return 0;
}

void MediaCounter::UnRegister(const std::string& uniqueID)
{
    if (m_stopped) {
        return;
    }

    LOG_INFO << "unregister uniqueID: " << uniqueID;

    CounterInfo ci;
    {
        std::lock_guard<std::mutex> guard(m_lock);
        auto iter = m_counters.find(uniqueID);
        if (iter == m_counters.end()) {
            LOG_ERROR << "can't find uniqueID: " << uniqueID;
            return;
        }

        ci = iter->second;
        m_counters.erase(uniqueID);
    }
    fclose(ci.m_file);
}

void MediaCounter::UpdateCounter(const std::string& uniqueID, long size)
{
    if (m_stopped) {
        return;
    }

    std::lock_guard<std::mutex> guard(m_lock);
    auto iter = m_counters.find(uniqueID);
    if (iter == m_counters.end()) {
        return;
    }
    iter->second.m_count += size;
}