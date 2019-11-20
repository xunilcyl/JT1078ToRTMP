#include "HttpNotifier.h"
#include "IConfiguration.h"
#include "Logger.h"
#include "SequenceGenerator.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <cassert>
#include <curl/curl.h>

using namespace boost::property_tree;

constexpr char ON_PLAY[] = "on_play";
constexpr char ON_STOP[] = "on_stop";
constexpr char ON_DISCONNECT[] = "on_disconnect";
constexpr int MAX_MSG_QUEUE_SIZE = 1024;

int HttpNotifier::Start()
{
    assert(m_thread == nullptr);
    LOG_INFO << "Start notifier";

    curl_global_init(CURL_GLOBAL_ALL);
    m_thread.reset(new std::thread(&HttpNotifier::Run, this));

    return 0;
}

void HttpNotifier::Stop()
{
    assert(m_thread);
    LOG_INFO << "Stop notifier";

    m_lock.lock();
    m_stopped = true;
    m_lock.unlock();
    m_condition.notify_all();

    m_thread->join();
    curl_global_cleanup();
}

void HttpNotifier::NotifyRtmpPlay(const std::string& uniqueID)
{
	NotifyAction(ON_PLAY, uniqueID);
}

void HttpNotifier::NotifyRtmpStop(const std::string& uniqueID)
{
    NotifyAction(ON_STOP, uniqueID);
}

void HttpNotifier::NotifyDeviceDisconnect(const std::string& uniqueID)
{
    NotifyAction(ON_DISCONNECT, uniqueID);
}

void HttpNotifier::NotifyAction(const char* action, const std::string& uniqueID)
{
    ptree pt;
	pt.put("action", action);
	pt.put("uniqueID", uniqueID.c_str());
	pt.put<int>("id", GenerateSequence());

	std::stringstream ss;
	json_parser::write_json(ss, pt);

	LOG_DEBUG << ss.str();

	Push(ss.str());
}

void HttpNotifier::Run()
{
    LOG_INFO << "Notifier thread is running.";

    std::string msg;

    while (true) {
        {
            boost::mutex::scoped_lock lock(m_lock);
            if (m_notifiesMsg.empty()) {
                m_condition.wait(lock);
            }

            if (m_stopped) {
                break;
            }

            msg = m_notifiesMsg.front();
            m_notifiesMsg.pop_front();
        }

        Send(msg);
    }

    LOG_INFO << "Notifier thread finished";
}

void HttpNotifier::Push(const std::string& msg)
{
    m_lock.lock();
    if (m_notifiesMsg.size() > MAX_MSG_QUEUE_SIZE) {
        LOG_ERROR << "Max number of messages which notifier can handle is " << MAX_MSG_QUEUE_SIZE;
        m_lock.unlock();
    }
    m_notifiesMsg.push_back(msg);
    m_lock.unlock();
    m_condition.notify_all();
}

void HttpNotifier::Send(const std::string& msg)
{
    LOG_DEBUG << "Send notify message";

    if (msg.empty()) {
        LOG_ERROR << "Can not send msg which is empty";
        return;
    }

    CURL* curl = curl_easy_init();
    if (curl == nullptr) {
        LOG_ERROR << "curl_easy_init failed";
        return;
    }

    curl_easy_setopt(curl, CURLOPT_URL, IConfiguration::Get().getHttpNotifyUrl().c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, msg.c_str());
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        LOG_ERROR << "curl_easy_perform failed";
    }

    curl_easy_cleanup(curl);

    LOG_DEBUG << "Send notify message done";
}
