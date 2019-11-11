#pragma once
#include "IMediaSession.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>

class MediaSession : public IMediaSession
{
public:
    MediaSession(boost::asio::ip::tcp::socket&& socket);

    int Start() override;
    void Stop() override;

private:
    void DoRead();

private:
    boost::asio::ip::tcp::socket m_socket;
    enum { MAX_BUFFER_SIZE = 4096 };
    char m_data[MAX_BUFFER_SIZE];
    boost::mutex m_lock;
    boost::condition_variable m_condition;
    bool m_stopped = false;
};