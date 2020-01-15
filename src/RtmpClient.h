#pragma once
#include "Common.h"
#include "IAudioSender.h"
#include "IRtmpClient.h"
#include "MediaBuffer.h"
#include "Packet.h"
#include <boost/thread.hpp>
#include <memory>
#include <string>
#include <thread>

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/time.h>
#include <libavutil/timestamp.h>
#ifdef __cplusplus
}
#endif

typedef std::shared_ptr<MediaBuffer> MediaBufferPtr;
typedef std::deque<MediaBufferPtr> MediaBufferQueue;
typedef std::deque<PacketType> PacketTypeQueue;

class IAudioSenderEngine;
class INotifier;

class RtmpClient : public IRtmpClient, public IAudioSender
{
public:
    RtmpClient(const std::string& uniqueID, INotifier& notifier, IAudioSenderEngine& audioSenderEngine);
    ~RtmpClient();

    int Start() override;
    int Stop() override;

    void OnData(const char* data, int size, bool isEndOfFrame) override;
    void OnAudioData(const char* data, int size, bool isEndOfFrame) override;

    void SendAudio();

private:
    static int ReadPacket(void* opaque, uint8* inputBuffer, int bufferSize);
    void Run();
    int ReadMediaData(uint8* inputBuffer, int bufferSize);

    void PublishStreamWithAvFormatContext();
    void PublishStreamWithParser();

    void TryToAddAudioStream(const std::string& rtmpUrl);


private:
    std::unique_ptr<std::thread> m_thread;
    boost::mutex m_lock;
    boost::mutex m_formatContextlock;
    boost::mutex m_audioLock;
    boost::condition_variable m_condition;
    MediaBufferQueue m_mediaBufferQueue;
    MediaBufferQueue m_audioBufferQueue;
    PacketTypeQueue m_packetTypeQueue;
    std::string m_uniqueID;
    INotifier& m_notifier;
    IAudioSenderEngine& m_audioSenderEngine;
    bool m_stopped = false;
    bool m_sendingVideoPacket = false;
    bool m_noAudio = false;
    bool m_audioReady = false;

    int m_getCount = 0;
    int m_sentCount = 0;
    FILE* m_file = NULL;
    AVFormatContext* m_outputFormatContext = nullptr;
    AVCodecParserContext* m_audioParser = nullptr;
    AVCodecContext* m_audioCodecContext = nullptr;
    AVFrame* m_audioFrame = nullptr;
    AVPacket m_audioPacket;
    int m_audioPacketCount = 0;
};
