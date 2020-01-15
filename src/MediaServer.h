#pragma once
#include "IMediaParser.h"
#include "IMediaServer.h"
#include "IMediaSession.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <memory>

class IMediaDataCallback;
class INotifier;

typedef std::unique_ptr<IMediaSession> MediaSessionPtr;
typedef std::unique_ptr<IMediaParser> MediaParserPtr;

class MediaServer : public IMediaServer, public IMediaSessionListener
{
public:
    MediaServer(
        boost::asio::io_context& ioContext,
        MediaParserPtr mediaParser,
        IMediaDataCallback& mediaDataCallback,
        INotifier& notifier,
        const std::string& uniqueID)
        : m_ioContext(ioContext)
        , m_acceptor(m_ioContext)
        , m_mediaParser(std::move(mediaParser))
        , m_mediaDataCallback(mediaDataCallback)
        , m_notifier(notifier)
        , m_uniqueID(uniqueID)
    {}

    int Start(int port) override;
    int Stop() override;

    // inherit from IMediaSessionListener
    void OnSessionError(const char* msg) override;

private:
    void DoAccept();

private:
    boost::asio::io_context& m_ioContext;
    boost::asio::ip::tcp::acceptor m_acceptor;
    MediaSessionPtr m_mediaSession;
    MediaParserPtr m_mediaParser;
    IMediaDataCallback& m_mediaDataCallback;
    INotifier& m_notifier;
    std::string m_uniqueID;

    boost::mutex m_lock;
    boost::condition_variable m_condition;
    bool m_stopped = false;
};