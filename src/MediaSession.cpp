#include "MediaSession.h"
#include "Logger.h"

MediaSession::MediaSession(boost::asio::ip::tcp::socket&& socket)
    : m_socket(std::move(socket))
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
        m_condition.wait(lock);
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
                return;
            }
            std::string dataRecv(m_data, m_data+length);
            LOG_INFO << "length: " << length << ", data: " << dataRecv.c_str();
            DoRead();
        });
}