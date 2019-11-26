#include "RtmpClient.h"
#include "Common.h"
#include "IConfiguration.h"
#include "INotifier.h"
#include "Logger.h"
#include "srs_librtmp.h"
#include <cassert>
#include <cstring>

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

constexpr char RTMP_URL_PREFIX[] = "rtmp://127.0.0.1/live/";

static void FFmpegLogCallback(void *ptr, int level, const char *fmt, va_list vl)
{
    char buf[512] = {0};
    vsprintf(buf, fmt, vl);

    switch (level) {
        case AV_LOG_TRACE:
        case AV_LOG_DEBUG:
        case AV_LOG_VERBOSE:
            LOG_DEBUG << buf;
            break;
        case AV_LOG_INFO:
            LOG_INFO << buf;
            break;
        case AV_LOG_WARNING:
            LOG_WARN << buf;
            break;
        case AV_LOG_ERROR:
        case AV_LOG_FATAL:
        case AV_LOG_PANIC:
            LOG_ERROR << buf;
            break;
        default:
            break;
    }
}

static void InitFFmpegLib()
{
    static bool s_isInit = false;
    if (!s_isInit) {
        //av_register_all();
        avformat_network_init();
        av_log_set_callback(FFmpegLogCallback);
        s_isInit = true;
    }
}

RtmpClient::RtmpClient(const std::string& uniqueID, INotifier& notifier)
    : m_uniqueID(uniqueID)
    , m_notifier(notifier)
{
    InitFFmpegLib();
}

int RtmpClient::Start()
{
    LOG_INFO << "Start RTMP client " << (void*)this;
    m_thread.reset(new std::thread(&RtmpClient::Run, this));

    return 0;
}

int RtmpClient::Stop()
{
    assert(m_thread);

    m_lock.lock();
    m_stopped = true;
    m_condition.notify_all();
    m_lock.unlock();

    m_thread->join();

    LOG_INFO << "RTMP client " << (void*)this << " stopped";

    return 0;
}

void RtmpClient::OnData(const char* data, int size)
{
    auto mb = std::make_shared<MediaBuffer>(data, size);

    m_lock.lock();
    int queueSize = m_mediaBufferQueue.size();
    if (queueSize >= 4096) {
        LOG_ERROR << (void*)this << " Media data buffer queue is full. Discard incoming data, size " << size;
    }
    else {
        m_mediaBufferQueue.push_back(std::move(mb));
    }
    m_condition.notify_all();
    m_lock.unlock();
}

// static
int RtmpClient::ReadPacket(void* opaque, uint8* inputBuffer, int bufferSize)
{
    assert(opaque);
    RtmpClient* pThis = static_cast<RtmpClient*>(opaque);

    return pThis->ReadMediaData(inputBuffer, bufferSize);
}

int RtmpClient::ReadMediaData(uint8* inputBuffer, int bufferSize)
{
    MediaBufferPtr mediaBuff;
    {
        boost::mutex::scoped_lock lock(m_lock);
        if (m_mediaBufferQueue.empty()) {
            m_condition.wait(lock);
        }
        if (m_stopped) {
            return AVERROR_EXIT;
            //return AVERROR_EOF;
        }
        mediaBuff = m_mediaBufferQueue.front();
        m_mediaBufferQueue.pop_front();
    }
    assert(mediaBuff);

    auto size = std::min(bufferSize, mediaBuff->m_size);
    memcpy(inputBuffer, mediaBuff->m_data, size);

    m_sentCount += size;

    return size;
}

// std::string ts2str(int64_t ts)
// {
//     char buf[32] = {0};
//     return av_ts_make_string(buf, ts);
// }

// std::string ts2timestr(int64_t ts, AVRational *tb)
// {
//     char buf[32] = {0};
//     return av_ts_make_time_string(buf, ts, tb);
// }

// static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag)
// {
//     AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

//     printf("%s: pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
//            tag,
//            ts2str(pkt->pts).c_str(), ts2timestr(pkt->pts, time_base).c_str(),
//            ts2str(pkt->dts).c_str(), ts2timestr(pkt->dts, time_base).c_str(),
//            ts2str(pkt->duration).c_str(), ts2timestr(pkt->duration, time_base).c_str(),
//            pkt->stream_index);
// }

void RtmpClient::Run()
{
    if (IConfiguration::Get().IfUseLibrtmp()) {
        LOG_INFO << "Publish stream with librtmp";
        PublishStreamWithParser();
    }
    else {
        LOG_INFO << "Publish stream with ffmpeg";
        PublishStreamWithAvFormatContext();
    }

    if (!m_stopped) {
        LOG_ERROR << "error occur when trying to publish stream for " << m_uniqueID;
        m_notifier.NotifyPublishError(m_uniqueID);
    }
}

void RtmpClient::PublishStreamWithAvFormatContext()
{
    AVFormatContext* inputFormatContext = avformat_alloc_context();
    if (!inputFormatContext) {
        LOG_ERROR << "avformat_alloc_context failed";
        return;
    }

    LOG_INFO << "this = " << (void*)this;
    uint8* inputBuffer = (uint8*)av_malloc(BUFFER_SIZE);
    AVIOContext* inputIoContext = avio_alloc_context(inputBuffer, BUFFER_SIZE, 0, (void*)this, &RtmpClient::ReadPacket, NULL, NULL);
    if (!inputIoContext) {
        avformat_free_context(inputFormatContext);
        av_free(inputBuffer);
        LOG_ERROR << "avio_alloc_context for input failed";
        return;
    }

    inputFormatContext->pb = inputIoContext;
    inputFormatContext->flags = AVFMT_FLAG_CUSTOM_IO;
    inputFormatContext->max_analyze_duration = IConfiguration::Get().GetMaxAnalyzeDuration();

    LOG_DEBUG << "max analyze duration: " << inputFormatContext->max_analyze_duration;

    if (avformat_open_input(&inputFormatContext, "", NULL, NULL) < 0) {
        avformat_free_context(inputFormatContext);
        avio_context_free(&inputIoContext);
        LOG_ERROR << "avformat_open_input failed";
        return;
    }
    LOG_INFO << "avformat_open_input success";

    if (avformat_find_stream_info(inputFormatContext, NULL) < 0) {
        avformat_close_input(&inputFormatContext);
        LOG_ERROR << "avformat_find_stream_info failed";
        return;
    }

    LOG_INFO << "find stream info success";
    

    int videoIndex = -1;
    for (int i = 0; i < (int)inputFormatContext->nb_streams; ++i) {
        if (inputFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIndex = i;
            break;
        }
    }

    if (videoIndex == -1) {
        avformat_close_input(&inputFormatContext);
        LOG_ERROR << "No video stream";
        return;
    }

    av_dump_format(inputFormatContext, videoIndex, "input", 0);
    
    std::string rtmpUrl = RTMP_URL_PREFIX + m_uniqueID;
    AVFormatContext* outputFormatContext = NULL;
    avformat_alloc_output_context2(&outputFormatContext, NULL, "flv", rtmpUrl.c_str());

    if (!outputFormatContext) {
        avformat_close_input(&inputFormatContext);
        LOG_ERROR << "avformat_alloc_output_context2 failed";
        return;
    }

    // TODO: add audio support soon

    //for (int j = 0; j < inputFormatContext->nb_streams; ++j) {
        AVStream* inputStream = inputFormatContext->streams[videoIndex];
        AVStream* outputStream = avformat_new_stream(outputFormatContext, inputStream->codec->codec);
        if (!outputStream) {
            avformat_close_input(&inputFormatContext);
            avformat_free_context(outputFormatContext);
            LOG_ERROR << "avformat_new_stream failed.";
            return;
        }

        if (avcodec_copy_context(outputStream->codec, inputStream->codec) < 0) {
            avformat_close_input(&inputFormatContext);
            avformat_free_context(outputFormatContext);
            LOG_ERROR << "avcodec_copy_context";
            return;
        }

        outputStream->codec->codec_tag = 0;
        if (outputFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
            outputStream->codec->flags |= AVFMT_GLOBALHEADER;
        }
    //}

    av_dump_format(outputFormatContext, videoIndex, rtmpUrl.c_str(), 1);

    if (avio_open(&outputFormatContext->pb, rtmpUrl.c_str(), AVIO_FLAG_WRITE) < 0) {
        avformat_close_input(&inputFormatContext);
        avformat_free_context(outputFormatContext);
        LOG_ERROR << "avio_open rtmp url failed";
        return;
    }

    LOG_INFO << "avio_open success";

    if (avformat_write_header(outputFormatContext, NULL) < 0) {
        avformat_close_input(&inputFormatContext);
        avio_close(outputFormatContext->pb);
        avformat_free_context(outputFormatContext);
        LOG_ERROR << "avformat_write_header failed";
        return;
    }

    LOG_INFO << "avformat_write_header success";

    //int64_t startTime = av_gettime();
    int frameIndex = 0;
    AVPacket packet;

    while (true) {
        if (av_read_frame(inputFormatContext, &packet)) {
            LOG_ERROR << "av_read_frame failed";
            break;
        }
        if (packet.stream_index != videoIndex) {
            LOG_INFO << "skip non-video packet";
            continue;
        }

        AVRational timeBase = inputFormatContext->streams[videoIndex]->time_base;
        int64_t duration = (double)AV_TIME_BASE / av_q2d(inputFormatContext->streams[videoIndex]->r_frame_rate);

        if (packet.pts == AV_NOPTS_VALUE) {
            packet.pts = (double)(frameIndex * duration) / (av_q2d(timeBase) * AV_TIME_BASE);
            packet.dts = packet.pts;
            packet.duration = (double)duration / (av_q2d(timeBase) * AV_TIME_BASE);
        }

        // no need delay when it is realplay
        // AVRational timeBaseQ = {1, AV_TIME_BASE};
        // int64_t ptsTime = av_rescale_q(packet.dts, timeBase, timeBaseQ);
        // int64_t nowTime = av_gettime() - startTime;
        // if (ptsTime > nowTime) {
        //     av_usleep(ptsTime - nowTime);
        // }

        packet.pts = av_rescale_q_rnd(packet.pts, inputStream->time_base, outputStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        packet.dts = av_rescale_q_rnd(packet.dts, inputStream->time_base, outputStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        packet.duration = av_rescale_q(packet.duration, inputStream->time_base, outputStream->time_base);
        packet.pos = -1;

        //log_packet(outputFormatContext, &packet, "out");

        if (av_interleaved_write_frame(outputFormatContext, &packet) < 0) {
            LOG_ERROR << "av_interleaved_write_frame failed";
            break;
        }

        av_free_packet(&packet);
        ++frameIndex;
    }

    av_write_trailer(outputFormatContext);

    avformat_close_input(&inputFormatContext);
    avio_close(outputFormatContext->pb);
    avformat_free_context(outputFormatContext);

    LOG_INFO << "Finish stream rtmp data";
}

void RtmpClient::PublishStreamWithParser()
{
    std::string rtmpUrl = RTMP_URL_PREFIX + m_uniqueID;

    srs_rtmp_t rtmp = srs_rtmp_create(rtmpUrl.c_str());

    if (!rtmp) {
        LOG_ERROR << "srs_rtmp_create failed.";
        return;
    }

    if (srs_rtmp_set_timeout(rtmp, 100000000, 100000000) < 0) {
        LOG_ERROR << "srs_rtmp_set_timeout failed";
        srs_rtmp_destroy(rtmp);
        return;
    }

    if (srs_rtmp_handshake(rtmp) != 0) {
        LOG_ERROR << "simple handshake failed.";
        srs_rtmp_destroy(rtmp);
        return;
    }

    LOG_INFO << "simple handshake success";
    
    if (srs_rtmp_connect_app(rtmp) != 0) {
        LOG_ERROR << "connect vhost/app failed.";
        srs_rtmp_destroy(rtmp);
        return;
    }
    LOG_INFO << "connect vhost/app success";
    
    if (srs_rtmp_publish_stream(rtmp) != 0) {
        LOG_ERROR << "publish stream failed.";
        srs_rtmp_destroy(rtmp);
        return;
    }
    LOG_INFO << "start to publish stream";

    AVCodec* inputCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!inputCodec) {
        LOG_ERROR << "avcodec_find_decoder h264 failed.";
        srs_rtmp_destroy(rtmp);
        return;
    }

    AVCodecParserContext* parser = av_parser_init(inputCodec->id);
    if (!parser) {
        LOG_ERROR << "av_parser_init failed.";
        srs_rtmp_destroy(rtmp);
        return;
    }

    AVCodecContext* inputCodecContext = avcodec_alloc_context3(inputCodec);
    if (!inputCodecContext) {
        LOG_ERROR << "avcodec_alloc_context3 failed";
        srs_rtmp_destroy(rtmp);
        av_parser_close(parser);
        return;
    }

    if (avcodec_open2(inputCodecContext, inputCodec, NULL) < 0) {
        LOG_ERROR << "avcodec_open2 failed";
        srs_rtmp_destroy(rtmp);
        av_parser_close(parser);
        avcodec_free_context(&inputCodecContext);
        return;
    }

    AVPacket packet;
    //int nPacket = 0;

    int dts = 0;
    int pts = 0;
    int fps = IConfiguration::Get().GetFps();

    while (true) {

        MediaBufferPtr mediaBuff;
        {
            boost::mutex::scoped_lock lock(m_lock);
            if (m_mediaBufferQueue.empty()) {
                m_condition.wait(lock);
            }
            if (m_stopped) {
                break;
            }
            mediaBuff = m_mediaBufferQueue.front();
            m_mediaBufferQueue.pop_front();
        }

        uint8* data = (uint8*)mediaBuff->m_data;
        int size = mediaBuff->m_size;

        while (size > 0) {
            auto used = av_parser_parse2(parser, inputCodecContext, &packet.data, &packet.size, data, size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
            if (used < 0) {
                LOG_ERROR << "av_parser_parse2 failed";
                srs_rtmp_destroy(rtmp);
                av_parser_close(parser);
                avcodec_free_context(&inputCodecContext);
                return;
            }
            data += used;
            size -= used;

            if (packet.size) {
                //LOG_DEBUG << "Get packet " << ++nPacket << ": data: " << (void*)packet.data << ", size: " << packet.size;
                //fwrite(packet.data, 1, packet.size, fp);
                pts += 1000 / fps;
                dts = pts;
                int ret = srs_h264_write_raw_frames(rtmp, (char*)packet.data, packet.size, dts, pts);
                if (ret != 0) {
                    if (srs_h264_is_dvbsp_error(ret)) {
                        LOG_DEBUG << "ignore drop video error, code=" << ret;
                    }
                    else if (srs_h264_is_duplicated_sps_error(ret)) {
                        LOG_DEBUG << "ignore duplicated sps, code=" << ret;
                    }
                    else if (srs_h264_is_duplicated_pps_error(ret)) {
                        LOG_DEBUG << "ignore duplicated pps, code=" << ret;
                    }
                    else {
                        LOG_ERROR << "send h264 raw data failed. ret=" << ret;
                        srs_rtmp_destroy(rtmp);
                        av_parser_close(parser);
                        avcodec_free_context(&inputCodecContext);
                        return;
                    }
                }
            }
        }
    }

    LOG_INFO << "Publish stream finished";

    srs_rtmp_destroy(rtmp);
    av_parser_close(parser);
    avcodec_free_context(&inputCodecContext);
}
