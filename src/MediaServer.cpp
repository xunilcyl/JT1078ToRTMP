#include "MediaServer.h"
#include "Logger.h"
#include "MediaSession.h"
#include <boost/bind.hpp>

using boost::asio::ip::tcp;

int MediaServer::Start(int port)
{
    LOG_INFO << "Start media server at port " << port;

    try {
        tcp::endpoint ep(tcp::v4(), port);
        m_acceptor.open(ep.protocol());
        m_acceptor.set_option(tcp::acceptor::reuse_address(true));
        m_acceptor.bind(ep);
        m_acceptor.listen();

        DoAccept();
    }
    catch (std::exception& e) {
        LOG_ERROR << "Start server at port " << port << " failed. " << e.what();
        return -1;
    }

    return 0;
}

int MediaServer::Stop()
{
    LOG_INFO << "Stop media server at port " << m_acceptor.local_endpoint().port();

    m_acceptor.close();
    
    if (m_mediaSession) {
        m_mediaSession->Stop();
    }

    boost::mutex::scoped_lock lock(m_lock);
    if (!m_stopped) {
        m_condition.wait(lock);
    }
    return 0;
}

void MediaServer::DoAccept()
{
    m_acceptor.async_accept(
        [this](const boost::system::error_code& error, tcp::socket socket)
        {
            if (!error) {
                auto remoteEp = socket.remote_endpoint();
                LOG_INFO << "client address: " << remoteEp.address().to_string() << ":" << remoteEp.port();
                
                if (m_mediaSession) {
                    LOG_WARN << "Only one device is allowed to connect to one media server port. Reject connection.";
                    socket.close();
                }
                else {
                    m_mediaSession.reset(new MediaSession(std::move(socket)));
                    m_mediaSession->Start();
                }
            }
            else {
                LOG_ERROR << "Accept connection failed. " << error.message();

                if (error.value() == boost::system::errc::operation_canceled) {
                    m_lock.lock();
                    m_stopped = true;
                    m_lock.unlock();
                    m_condition.notify_all();
                    return;
                }
            }
            DoAccept();
        });
}
