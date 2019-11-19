#pragma once
#include "INotifier.h"
#include <boost/thread.hpp>
#include <deque>
#include <memory>
#include <thread>

class HttpNotifier : public INotifier
{
public:

    int Start() override;
    void Stop() override;
    void NotifyRtmpPlay(const std::string& uniqueID) override;
    void NotifyRtmpStop(const std::string& uniqueID) override;
    void NotifyDeviceDisconnect(const std::string& uniqueID) override;

private:
    void Run();
    void Push(const std::string& msg);
    void Send(const std::string& msg);
    void NotifyAction(const char* action, const std::string& uniqueID);

private:
    std::unique_ptr<std::thread> m_thread;
    std::deque<std::string> m_notifiesMsg;
    boost::mutex m_lock;
    boost::condition_variable m_condition;
    bool m_stopped = false;
};