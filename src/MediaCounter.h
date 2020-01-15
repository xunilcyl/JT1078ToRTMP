#pragma once

#include "IMediaCounter.h"
#include "Common.h"
#include <cstdio>
#include <mutex>
#include <thread>
#include <unordered_map>

class CounterInfo
{
public:
    FILE* m_file = NULL;
    uint64 m_count = 0;
};

class MediaCounter : public IMediaCounter
{
public:
    MediaCounter();
    virtual ~MediaCounter();
    int Register(const std::string& uniqueID) override;
    void UnRegister(const std::string& uniqueID) override;
    void UpdateCounter(const std::string& uniqueID, long size) override;

private:
    void Run();
    void Start();
    void Stop();
    void StoreRecord(CounterInfo& info, time_t curTime);

private:
    std::unique_ptr<std::thread> m_thread;
    std::unordered_map<std::string, CounterInfo> m_counters;
    std::mutex m_lock;
    bool m_stopped = true;
};