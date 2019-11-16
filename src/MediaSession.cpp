#include "MediaSession.h"
#include "IMediaDataCallback.h"
#include "IMediaParser.h"
#include "Logger.h"

MediaSession::MediaSession(boost::asio::ip::tcp::socket&& socket, IMediaParser& mediaParser, IMediaDataCallback& mediaDataCallback, IMediaSessionListener& listener)
    : m_socket(std::move(socket))
    , m_mediaParser(mediaParser)
    , m_mediaDataCallback(mediaDataCallback)
    , m_mediaSessionListener(listener)
{
}

int MediaSession::Start()
{
    LOG_INFO << "Start media session";
    DoRead();
    return 0;
}

void MediaSession::Stop()
{
    LOG_INFO << "Stop media session";
    m_socket.close();
    boost::mutex::scoped_lock lock(m_lock);
    if (!m_stopped) {
        LOG_INFO << "waiting to media session stop";
        m_condition.wait(lock);
        LOG_INFO << "get notify. media session end. continue";
    }
}

void MediaSession::DoRead()
{
    m_socket.async_read_some(
        boost::asio::buffer(m_data, sizeof(m_data)),
        [this](boost::system::error_code ec, std::size_t length)
        {
            if (ec) {
                auto localEp = m_socket.remote_endpoint();
                LOG_ERROR << "Fail to read data from " << localEp.address().to_string() << ":" << localEp.port() << ", " << ec.message();

                m_lock.lock();
                m_stopped = true;
                m_lock.unlock();
                m_condition.notify_all();

                if (ec.value() != boost::system::errc::operation_canceled) {
                    m_mediaSessionListener.OnError(ec.message().c_str());
                }
                return;
            }

            ParseData(m_data, length);

            DoRead();
        });
}

int MediaSession::ParseData(const char* data, int length)
{
    if (m_mediaParser.Parse(data, length) < 0) {
        return -1;
    }

    Packet* packet = m_mediaParser.AllocatePacket();
    
    while (m_mediaParser.GetPacket(packet) >= 0) {
        if (packet->m_type == PACKET_VIDEO) {
            //LOG_DEBUG << "Video packet size " << packet->m_size;
            m_mediaDataCallback.OnData(packet->m_data, packet->m_size);
        }
        else {
            LOG_DEBUG << "Get packet which type is " << packet->m_type << ", size is " << packet->m_size;
        }
    }

    m_mediaParser.FreePacket(packet);

    return 0;
}