#pragma once
#include "IMediaServer.h"
#include "IMediaSession.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <memory>

typedef std::unique_ptr<IMediaSession> MediaSessionPtr;

class MediaServer : public IMediaServer
{
public:
    MediaServer(boost::asio::io_context& ioContext)
        : m_ioContext(ioContext)
        , m_acceptor(m_ioContext)
    {}

    int Start(int port) override;
    int Stop() override;

private:
    void DoAccept();

private:
    boost::asio::io_context& m_ioContext;
    boost::asio::ip::tcp::acceptor m_acceptor;
    MediaSessionPtr m_mediaSession;

    boost::mutex m_lock;
    boost::condition_variable m_condition;
    bool m_stopped = false;
};