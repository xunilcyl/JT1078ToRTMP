#pragma once
#include "Common.h"
#include "IRtmpClient.h"
#include "MediaBuffer.h"
#include <boost/thread.hpp>
#include <memory>
#include <string>
#include <thread>

typedef std::shared_ptr<MediaBuffer> MediaBufferPtr;
typedef std::deque<MediaBufferPtr> MediaBufferQueue;

class RtmpClient : public IRtmpClient
{
public:
    RtmpClient(const std::string& uniqueID);

    int Start() override;
    int Stop() override;

    void OnData(const char* data, int size) override;

private:
    static int ReadPacket(void* opaque, uint8* inputBuffer, int bufferSize);
    void Run();
    int ReadMediaData(uint8* inputBuffer, int bufferSize);

private:
    std::unique_ptr<std::thread> m_thread;
    boost::mutex m_lock;
    boost::condition_variable m_condition;
    MediaBufferQueue m_mediaBufferQueue;
    std::string m_uniqueID;
    bool m_stopped = false;

    int m_getCount = 0;
    int m_sentCount = 0;
    FILE* m_file = NULL;
};