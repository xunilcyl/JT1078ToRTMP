#pragma once
#include "IMediaSession.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>

class IMediaParser;
class IMediaDataCallback;

class MediaSession : public IMediaSession
{
public:
    MediaSession(boost::asio::ip::tcp::socket&& socket, IMediaParser& mediaParser, IMediaDataCallback& mediaDataCallback, IMediaSessionListener& listener);

    int Start() override;
    void Stop() override;

private:
    void DoRead();
    int ParseData(const char* data, int length);

private:
    boost::asio::ip::tcp::socket m_socket;
    IMediaParser& m_mediaParser;
    IMediaDataCallback& m_mediaDataCallback;
    IMediaSessionListener& m_mediaSessionListener;
    enum { MAX_BUFFER_SIZE = 4096 };
    char m_data[MAX_BUFFER_SIZE];
    boost::mutex m_lock;
    boost::condition_variable m_condition;
    bool m_stopped = false;
};